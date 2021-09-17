#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
//
//   fxc /nologo /E VS_Multiview_Clear /T vs_4_0 /Fh
//    compiled\clear11multiviewvs.h Clear11.hlsl
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue Format   Used
// -------------------- ----- ------ -------- -------- ------ ------
// SV_VertexID              0   x           0   VERTID   uint   x   
// SV_InstanceID            0   x           1   INSTID   uint   x   
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue Format   Used
// -------------------- ----- ------ -------- -------- ------ ------
// SV_POSITION              0   xyzw        0      POS  float   xyzw
// TEXCOORD                 0   x           1     NONE   uint   x   
//
vs_4_0
dcl_immediateConstantBuffer { { -1.000000, 1.000000, 0, 0},
                              { 1.000000, -1.000000, 0, 0},
                              { -1.000000, -1.000000, 0, 0},
                              { -1.000000, 1.000000, 0, 0},
                              { 1.000000, 1.000000, 0, 0},
                              { 1.000000, -1.000000, 0, 0} }
dcl_input_sgv v0.x, vertex_id
dcl_input_sgv v1.x, instance_id
dcl_output_siv o0.xyzw, position
dcl_output o1.x
dcl_temps 1
mov r0.x, v0.x
mov o0.xy, icb[r0.x + 0].xyxx
mov o0.zw, l(0,0,0,1.000000)
mov o1.x, v1.x
ret 
// Approximately 5 instruction slots used
#endif

const BYTE g_VS_Multiview_Clear[] = {
    68,  88,  66,  67,  14,  99,  210, 81,  182, 39,  8,   134, 74,  169, 88,  103, 114, 197, 56,
    220, 1,   0,   0,   0,   220, 2,   0,   0,   5,   0,   0,   0,   52,  0,   0,   0,   140, 0,
    0,   0,   232, 0,   0,   0,   64,  1,   0,   0,   96,  2,   0,   0,   82,  68,  69,  70,  80,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   28,  0,   0,   0,
    0,   4,   254, 255, 0,   1,   0,   0,   28,  0,   0,   0,   77,  105, 99,  114, 111, 115, 111,
    102, 116, 32,  40,  82,  41,  32,  72,  76,  83,  76,  32,  83,  104, 97,  100, 101, 114, 32,
    67,  111, 109, 112, 105, 108, 101, 114, 32,  57,  46,  50,  57,  46,  57,  53,  50,  46,  51,
    49,  49,  49,  0,   171, 171, 171, 73,  83,  71,  78,  84,  0,   0,   0,   2,   0,   0,   0,
    8,   0,   0,   0,   56,  0,   0,   0,   0,   0,   0,   0,   6,   0,   0,   0,   1,   0,   0,
    0,   0,   0,   0,   0,   1,   1,   0,   0,   68,  0,   0,   0,   0,   0,   0,   0,   8,   0,
    0,   0,   1,   0,   0,   0,   1,   0,   0,   0,   1,   1,   0,   0,   83,  86,  95,  86,  101,
    114, 116, 101, 120, 73,  68,  0,   83,  86,  95,  73,  110, 115, 116, 97,  110, 99,  101, 73,
    68,  0,   171, 171, 79,  83,  71,  78,  80,  0,   0,   0,   2,   0,   0,   0,   8,   0,   0,
    0,   56,  0,   0,   0,   0,   0,   0,   0,   1,   0,   0,   0,   3,   0,   0,   0,   0,   0,
    0,   0,   15,  0,   0,   0,   68,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,
    0,   0,   0,   1,   0,   0,   0,   1,   14,  0,   0,   83,  86,  95,  80,  79,  83,  73,  84,
    73,  79,  78,  0,   84,  69,  88,  67,  79,  79,  82,  68,  0,   171, 171, 171, 83,  72,  68,
    82,  24,  1,   0,   0,   64,  0,   1,   0,   70,  0,   0,   0,   53,  24,  0,   0,   26,  0,
    0,   0,   0,   0,   128, 191, 0,   0,   128, 63,  0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   128, 63,  0,   0,   128, 191, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   128, 191,
    0,   0,   128, 191, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   128, 191, 0,   0,   128,
    63,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   128, 63,  0,   0,   128, 63,  0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   128, 63,  0,   0,   128, 191, 0,   0,   0,   0,   0,
    0,   0,   0,   96,  0,   0,   4,   18,  16,  16,  0,   0,   0,   0,   0,   6,   0,   0,   0,
    96,  0,   0,   4,   18,  16,  16,  0,   1,   0,   0,   0,   8,   0,   0,   0,   103, 0,   0,
    4,   242, 32,  16,  0,   0,   0,   0,   0,   1,   0,   0,   0,   101, 0,   0,   3,   18,  32,
    16,  0,   1,   0,   0,   0,   104, 0,   0,   2,   1,   0,   0,   0,   54,  0,   0,   5,   18,
    0,   16,  0,   0,   0,   0,   0,   10,  16,  16,  0,   0,   0,   0,   0,   54,  0,   0,   6,
    50,  32,  16,  0,   0,   0,   0,   0,   70,  144, 144, 0,   10,  0,   16,  0,   0,   0,   0,
    0,   54,  0,   0,   8,   194, 32,  16,  0,   0,   0,   0,   0,   2,   64,  0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   128, 63,  54,  0,   0,   5,   18,
    32,  16,  0,   1,   0,   0,   0,   10,  16,  16,  0,   1,   0,   0,   0,   62,  0,   0,   1,
    83,  84,  65,  84,  116, 0,   0,   0,   5,   0,   0,   0,   1,   0,   0,   0,   6,   0,   0,
    0,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0};
