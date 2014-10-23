/*
 * Copyright (C) 2013 University of Washington. All rights reserved.
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

WebInspector.ProbeSample = function(sampleId, batchId, timestamp, payload)
{
    this.sampleId = sampleId;
    this.batchId = batchId;
    this.timestamp = timestamp;
    this.object = WebInspector.RemoteObject.fromPayload(payload);
};

WebInspector.Probe = function(id, breakpoint, expression)
{
    WebInspector.Object.call(this);

    console.assert(id);
    console.assert(breakpoint instanceof WebInspector.Breakpoint);

    this._id = id;
    this._breakpoint = breakpoint;
    this._expression = expression;
    this._samples = [];
};

WebInspector.Object.addConstructorFunctions(WebInspector.Probe);

WebInspector.Probe.Event = {
    ExpressionChanged: "probe-object-expression-changed",
    SampleAdded: "probe-object-sample-added",
    SamplesCleared: "probe-object-samples-cleared"
};

WebInspector.Probe.prototype = {
    constructor: WebInspector.Probe,
    __proto__: WebInspector.Object.prototype,

    // Public

    get id()
    {
        return this._id;
    },

    get breakpoint()
    {
        return this._breakpoint;
    },

    get expression()
    {
        return this._expression;
    },

    set expression(value)
    {
        if (this._expression === value)
            return;

        var data = {oldValue: this._expression, newValue: value};
        this._expression = value;
        this.clearSamples();
        this.dispatchEventToListeners(WebInspector.Probe.Event.ExpressionChanged, data);
    },

    get samples()
    {
        return this._samples.slice();
    },

    clearSamples: function()
    {
        this._samples = [];
        this.dispatchEventToListeners(WebInspector.Probe.Event.SamplesCleared);
    },

    addSample: function(sample)
    {
        console.assert(sample instanceof WebInspector.ProbeSample, "Wrong object type passed as probe sample: ", sample);
        this._samples.push(sample);
        this.dispatchEventToListeners(WebInspector.Probe.Event.SampleAdded, sample);
    }
};
