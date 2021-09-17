/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003-2019 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#pragma once

#include "BatchedTransitionOptimizer.h"
#include "CallFrame.h"
#include "CustomGetterSetter.h"
#include "DOMJITGetterSetter.h"
#include "DOMJITSignature.h"
#include "Identifier.h"
#include "IdentifierInlines.h"
#include "Intrinsic.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "LazyProperty.h"
#include "PropertySlot.h"
#include "PutPropertySlot.h"
#include "TypeError.h"
#include <wtf/Assertions.h>

namespace JSC {

struct CompactHashIndex {
    const int16_t value;
    const int16_t next;
};

// FIXME: There is no reason this get function can't be simpler.
// ie. typedef JSValue (*GetFunction)(JSGlobalObject*, JSObject* baseObject)
typedef PropertySlot::GetValueFunc GetFunction;
typedef PutPropertySlot::PutValueFunc PutFunction;
typedef FunctionExecutable* (*BuiltinGenerator)(VM&);
typedef JSValue (*LazyPropertyCallback)(VM&, JSObject*);

// Hash table generated by the create_hash_table script.
struct HashTableValue {
    const char* m_key; // property name
    unsigned m_attributes; // JSObject attributes
    Intrinsic m_intrinsic;
    union ValueStorage {
        constexpr ValueStorage(intptr_t value1, intptr_t value2)
            : value1(value1)
            , value2(value2)
        { }
        constexpr ValueStorage(long long constant)
            : constant(constant)
        { }

        struct {
            intptr_t value1;
            intptr_t value2;
        };
        long long constant;
    } m_values;

    unsigned attributes() const { return m_attributes; }

    Intrinsic intrinsic() const { ASSERT(m_attributes & PropertyAttribute::Function); return m_intrinsic; }
    BuiltinGenerator builtinGenerator() const { ASSERT(m_attributes & PropertyAttribute::Builtin); return reinterpret_cast<BuiltinGenerator>(m_values.value1); }
    NativeFunction function() const { ASSERT(m_attributes & PropertyAttribute::Function); return NativeFunction(m_values.value1); }
    unsigned char functionLength() const
    {
        ASSERT(m_attributes & PropertyAttribute::Function);
        if (m_attributes & PropertyAttribute::DOMJITFunction)
            return signature()->argumentCount;
        return static_cast<unsigned char>(m_values.value2);
    }

    GetFunction propertyGetter() const { ASSERT(!(m_attributes & PropertyAttribute::BuiltinOrFunctionOrAccessorOrLazyPropertyOrConstant)); return reinterpret_cast<GetFunction>(m_values.value1); }
    PutFunction propertyPutter() const { ASSERT(!(m_attributes & PropertyAttribute::BuiltinOrFunctionOrAccessorOrLazyPropertyOrConstant)); return reinterpret_cast<PutFunction>(m_values.value2); }

    const DOMJIT::GetterSetter* domJIT() const { ASSERT(m_attributes & PropertyAttribute::DOMJITAttribute); return reinterpret_cast<const DOMJIT::GetterSetter*>(m_values.value1); }
    const DOMJIT::Signature* signature() const { ASSERT(m_attributes & PropertyAttribute::DOMJITFunction); return reinterpret_cast<const DOMJIT::Signature*>(m_values.value2); }

    NativeFunction accessorGetter() const { ASSERT(m_attributes & PropertyAttribute::Accessor); return NativeFunction(m_values.value1); }
    NativeFunction accessorSetter() const { ASSERT(m_attributes & PropertyAttribute::Accessor); return NativeFunction(m_values.value2); }
    BuiltinGenerator builtinAccessorGetterGenerator() const;
    BuiltinGenerator builtinAccessorSetterGenerator() const;

    long long constantInteger() const { ASSERT(m_attributes & PropertyAttribute::ConstantInteger); return m_values.constant; }

    intptr_t lexerValue() const { ASSERT(!m_attributes); return m_values.value1; }
    
    ptrdiff_t lazyCellPropertyOffset() const { ASSERT(m_attributes & PropertyAttribute::CellProperty); return m_values.value1; }
    ptrdiff_t lazyClassStructureOffset() const { ASSERT(m_attributes & PropertyAttribute::ClassStructure); return m_values.value1; }
    LazyPropertyCallback lazyPropertyCallback() const { ASSERT(m_attributes & PropertyAttribute::PropertyCallback); return reinterpret_cast<LazyPropertyCallback>(m_values.value1); }
};

struct HashTable {
    int numberOfValues;
    int indexMask;
    bool hasSetterOrReadonlyProperties;
    const ClassInfo* classForThis; // Used by DOMAttribute. Attribute accessors perform type check against this classInfo.

    const HashTableValue* values; // Fixed values generated by script.
    const CompactHashIndex* index;

    // Find an entry in the table, and return the entry.
    ALWAYS_INLINE const HashTableValue* entry(PropertyName propertyName) const
    {
        if (propertyName.isSymbol())
            return nullptr;

        auto uid = propertyName.uid();
        if (!uid)
            return nullptr;

        int indexEntry = IdentifierRepHash::hash(uid) & indexMask;
        int valueIndex = index[indexEntry].value;
        if (valueIndex == -1)
            return nullptr;

        while (true) {
            if (WTF::equal(uid, values[valueIndex].m_key))
                return &values[valueIndex];

            indexEntry = index[indexEntry].next;
            if (indexEntry == -1)
                return nullptr;
            valueIndex = index[indexEntry].value;
            ASSERT(valueIndex != -1);
        };
    }

    class ConstIterator {
    public:
        ConstIterator(const HashTable* table, int position)
            : m_table(table)
            , m_position(position)
        {
            skipInvalidKeys();
        }

        const HashTableValue* value() const
        {
            return &m_table->values[m_position];
        }

        const HashTableValue& operator*() const { return *value(); }

        const char* key() const
        {
            return m_table->values[m_position].m_key;
        }

        const HashTableValue* operator->() const
        {
            return value();
        }

        bool operator!=(const ConstIterator& other) const
        {
            ASSERT(m_table == other.m_table);
            return m_position != other.m_position;
        }

        ConstIterator& operator++()
        {
            ASSERT(m_position < m_table->numberOfValues);
            ++m_position;
            skipInvalidKeys();
            return *this;
        }

    private:
        void skipInvalidKeys()
        {
            ASSERT(m_position <= m_table->numberOfValues);
            while (m_position < m_table->numberOfValues && !m_table->values[m_position].m_key)
                ++m_position;
            ASSERT(m_position <= m_table->numberOfValues);
        }

        const HashTable* m_table;
        int m_position;
    };

    ConstIterator begin() const
    {
        return ConstIterator(this, 0);
    }
    ConstIterator end() const
    {
        return ConstIterator(this, numberOfValues);
    }
};

JS_EXPORT_PRIVATE bool setUpStaticFunctionSlot(VM&, const ClassInfo*, const HashTableValue*, JSObject* thisObject, PropertyName, PropertySlot&);
JS_EXPORT_PRIVATE void reifyStaticAccessor(VM&, const HashTableValue&, JSObject& thisObject, PropertyName);

inline BuiltinGenerator HashTableValue::builtinAccessorGetterGenerator() const
{
    ASSERT(m_attributes & PropertyAttribute::Accessor);
    ASSERT(m_attributes & PropertyAttribute::Builtin);
    return reinterpret_cast<BuiltinGenerator>(m_values.value1);
}

inline BuiltinGenerator HashTableValue::builtinAccessorSetterGenerator() const
{
    ASSERT(m_attributes & PropertyAttribute::Accessor);
    ASSERT(m_attributes & PropertyAttribute::Builtin);
    return reinterpret_cast<BuiltinGenerator>(m_values.value2);
}

inline bool getStaticPropertySlotFromTable(VM& vm, const ClassInfo* classInfo, const HashTable& table, JSObject* thisObject, PropertyName propertyName, PropertySlot& slot)
{
    if (thisObject->staticPropertiesReified(vm))
        return false;

    auto* entry = table.entry(propertyName);
    if (!entry)
        return false;

    if (entry->attributes() & PropertyAttribute::BuiltinOrFunctionOrAccessorOrLazyProperty)
        return setUpStaticFunctionSlot(vm, classInfo, entry, thisObject, propertyName, slot);

    if (entry->attributes() & PropertyAttribute::ConstantInteger) {
        slot.setValue(thisObject, attributesForStructure(entry->attributes()), jsNumber(entry->constantInteger()));
        return true;
    }

    if (entry->attributes() & PropertyAttribute::DOMJITAttribute) {
        ASSERT_WITH_MESSAGE(entry->attributes() & PropertyAttribute::ReadOnly, "DOMJITAttribute supports readonly attributes currently.");
        const DOMJIT::GetterSetter* domJIT = entry->domJIT();
        slot.setCacheableCustom(thisObject, attributesForStructure(entry->attributes()), domJIT->getter(), nullptr, DOMAttributeAnnotation { classInfo, domJIT });
        return true;
    }

    if (entry->attributes() & PropertyAttribute::DOMAttribute) {
        slot.setCacheableCustom(thisObject, attributesForStructure(entry->attributes()), entry->propertyGetter(), entry->propertyPutter(), DOMAttributeAnnotation { classInfo, nullptr });
        return true;
    }

    slot.setCacheableCustom(thisObject, attributesForStructure(entry->attributes()), entry->propertyGetter(), entry->propertyPutter());
    return true;
}

inline bool replaceStaticPropertySlot(VM& vm, JSObject* thisObject, PropertyName propertyName, JSValue value)
{
    if (!thisObject->putDirect(vm, propertyName, value))
        return false;

    if (!thisObject->staticPropertiesReified(vm))
        thisObject->JSObject::setStructure(vm, Structure::attributeChangeTransition(vm, thisObject->structure(vm), propertyName, 0));

    return true;
}

// 'base' means the object holding the property (possibly in the prototype chain of the object put was called on).
// 'thisValue' is the object that put is being applied to (in the case of a proxy, the proxy target).
// 'slot.thisValue()' is the object the put was originally performed on (in the case of a proxy, the proxy itself).
inline bool putEntry(JSGlobalObject* globalObject, const ClassInfo*, const HashTableValue* entry, JSObject* base, JSObject* thisValue, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    VM& vm = getVM(globalObject);
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (entry->attributes() & PropertyAttribute::BuiltinOrFunctionOrLazyProperty) {
        if (!(entry->attributes() & PropertyAttribute::ReadOnly)) {
            // If this is a function or lazy property put then we just do the put because
            // logically the object already had the property, so this is just a replace.
            // FIXME: thisValue may be an object with non-extensible structure we must throw for.
            if (JSObject* thisObject = jsDynamicCast<JSObject*>(vm, thisValue))
                thisObject->putDirect(vm, propertyName, value);
            return true;
        }
        return typeError(globalObject, scope, slot.isStrictMode(), ReadonlyPropertyWriteError);
    }

    if (entry->attributes() & PropertyAttribute::Accessor)
        return typeError(globalObject, scope, slot.isStrictMode(), ReadonlyPropertyWriteError);

    if (!(entry->attributes() & PropertyAttribute::ReadOnly)) {
        ASSERT_WITH_MESSAGE(!(entry->attributes() & PropertyAttribute::DOMJITAttribute), "DOMJITAttribute supports readonly attributes currently.");
        bool isAccessor = entry->attributes() & PropertyAttribute::CustomAccessor;
        // FIXME: We should only be caching these if we're not an uncacheable dictionary:
        // https://bugs.webkit.org/show_bug.cgi?id=215347
        // We need to make sure that we decide to cache this property before we potentially execute aribitrary JS.
        if (isAccessor)
            slot.setCustomAccessor(base, entry->propertyPutter());
        else
            slot.setCustomValue(base, entry->propertyPutter());

        auto result = callCustomSetter(globalObject, entry->propertyPutter(), isAccessor, base, slot.thisValue(), value);
        RETURN_IF_EXCEPTION(scope, false);
        if (result != TriState::Indeterminate)
            return result == TriState::True;

        // FIXME: thisValue may be an object with non-extensible structure we must throw for.
        if (JSObject* thisObject = jsDynamicCast<JSObject*>(vm, thisValue))
            thisObject->putDirect(vm, propertyName, value);
        return true;
    }

    return typeError(globalObject, scope, slot.isStrictMode(), ReadonlyPropertyWriteError);
}

/**
 * This one is for "put".
 * It looks up a hash entry for the property to be set.  If an entry
 * is found it sets the value and returns true, else it returns false.
 */
inline bool lookupPut(JSGlobalObject* globalObject, PropertyName propertyName, JSObject* base, JSValue value, const HashTable& table, PutPropertySlot& slot, bool& putResult)
{
    const HashTableValue* entry = table.entry(propertyName);

    if (!entry)
        return false;

    putResult = putEntry(globalObject, table.classForThis, entry, base, base, propertyName, value, slot);
    return true;
}

inline void reifyStaticProperty(VM& vm, const ClassInfo* classInfo, const PropertyName& propertyName, const HashTableValue& value, JSObject& thisObj)
{
    if (value.attributes() & PropertyAttribute::Builtin) {
        if (value.attributes() & PropertyAttribute::Accessor)
            reifyStaticAccessor(vm, value, thisObj, propertyName);
        else
            thisObj.putDirectBuiltinFunction(vm, thisObj.globalObject(vm), propertyName, value.builtinGenerator()(vm), attributesForStructure(value.attributes()));
        return;
    }

    if (value.attributes() & PropertyAttribute::Function) {
        if (value.attributes() & PropertyAttribute::DOMJITFunction) {
            thisObj.putDirectNativeFunction(
                vm, thisObj.globalObject(vm), propertyName, value.functionLength(),
                value.function(), value.intrinsic(), value.signature(), attributesForStructure(value.attributes()));
            return;
        }
        thisObj.putDirectNativeFunction(
            vm, thisObj.globalObject(vm), propertyName, value.functionLength(),
            value.function(), value.intrinsic(), attributesForStructure(value.attributes()));
        return;
    }

    if (value.attributes() & PropertyAttribute::ConstantInteger) {
        thisObj.putDirect(vm, propertyName, jsNumber(value.constantInteger()), attributesForStructure(value.attributes()));
        return;
    }

    if (value.attributes() & PropertyAttribute::Accessor) {
        reifyStaticAccessor(vm, value, thisObj, propertyName);
        return;
    }
    
    if (value.attributes() & PropertyAttribute::CellProperty) {
        LazyCellProperty* property = bitwise_cast<LazyCellProperty*>(
            bitwise_cast<char*>(&thisObj) + value.lazyCellPropertyOffset());
        JSCell* result = property->get(&thisObj);
        thisObj.putDirect(vm, propertyName, result, attributesForStructure(value.attributes()));
        return;
    }
    
    if (value.attributes() & PropertyAttribute::ClassStructure) {
        LazyClassStructure* lazyStructure = bitwise_cast<LazyClassStructure*>(
            bitwise_cast<char*>(&thisObj) + value.lazyClassStructureOffset());
        JSObject* constructor = lazyStructure->constructor(jsCast<JSGlobalObject*>(&thisObj));
        thisObj.putDirect(vm, propertyName, constructor, attributesForStructure(value.attributes()));
        return;
    }
    
    if (value.attributes() & PropertyAttribute::PropertyCallback) {
        JSValue result = value.lazyPropertyCallback()(vm, &thisObj);
        thisObj.putDirect(vm, propertyName, result, attributesForStructure(value.attributes()));
        return;
    }

    if (value.attributes() & PropertyAttribute::DOMJITAttribute) {
        ASSERT_WITH_MESSAGE(classInfo, "DOMJITAttribute should have class info for type checking.");
        const DOMJIT::GetterSetter* domJIT = value.domJIT();
        auto* customGetterSetter = DOMAttributeGetterSetter::create(vm, domJIT->getter(), value.propertyPutter(), DOMAttributeAnnotation { classInfo, domJIT });
        thisObj.putDirectCustomAccessor(vm, propertyName, customGetterSetter, attributesForStructure(value.attributes()));
        return;
    }

    if (value.attributes() & PropertyAttribute::DOMAttribute) {
        ASSERT_WITH_MESSAGE(classInfo, "DOMAttribute should have class info for type checking.");
        auto* customGetterSetter = DOMAttributeGetterSetter::create(vm, value.propertyGetter(), value.propertyPutter(), DOMAttributeAnnotation { classInfo, nullptr });
        thisObj.putDirectCustomAccessor(vm, propertyName, customGetterSetter, attributesForStructure(value.attributes()));
        return;
    }

    CustomGetterSetter* customGetterSetter = CustomGetterSetter::create(vm, value.propertyGetter(), value.propertyPutter());
    thisObj.putDirectCustomAccessor(vm, propertyName, customGetterSetter, attributesForStructure(value.attributes()));
}

template<unsigned numberOfValues>
inline void reifyStaticProperties(VM& vm, const ClassInfo* classInfo, const HashTableValue (&values)[numberOfValues], JSObject& thisObj)
{
    BatchedTransitionOptimizer transitionOptimizer(vm, &thisObj);
    for (auto& value : values) {
        if (!value.m_key)
            continue;
        auto key = Identifier::fromString(vm, reinterpret_cast<const LChar*>(value.m_key), strlen(value.m_key));
        reifyStaticProperty(vm, classInfo, key, value, thisObj);
    }
}

template<RawNativeFunction nativeFunction, int length> EncodedJSValue nonCachingStaticFunctionGetterImpl(JSGlobalObject* globalObject, PropertyName propertyName)
{
    return JSValue::encode(JSFunction::create(globalObject->vm(), globalObject, length, propertyName.publicName(), nativeFunction));
}

} // namespace JSC
