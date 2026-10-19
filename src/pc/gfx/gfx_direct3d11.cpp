#if defined(_WIN32)

#include <cstdio>
#include <vector>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <windows.h>
#include <versionhelpers.h>
#include <wrl/client.h>

#include <dxgi1_3.h>
#include <d3d11.h>
#include <d3dcompiler.h>

extern "C" {
#include "gfx_pc.h"
#include "gfx_rendering_api.h"
}



static const char D3D11_UBERSHADER[] = {
#embed "gfx_cc.inl"
,
#embed "shaders/direct3d11/ubershader.vs.hlsl"
,
#embed "shaders/direct3d11/ubershader.ps.hlsl"
,0
};


/*
struct PSInput {
    float4 position : SV_POSITION;
    float2 uv0 : TEXCOORD0;    float4 input1 : INPUT0;
    float4 input2 : INPUT1;
};
Texture2D g_texture0 : register(t0);
SamplerState g_sampler0 : register(s0);
cbuffer PerDrawCB : register(b1) {
    struct {
        uint width;
        uint height;
        bool linear_filtering;
        bool padding;
    } textures[2];
    uint filter;
    uint padding[3];
}
#define TEX_OFFSET(tex, tSampler, texCoord, off, texSize) tex.Sample(tSampler, texCoord - off / texSize)
float4 tex2D3PointFilter(in Texture2D tex, in SamplerState tSampler, in float2 texCoord, in float2 texSize) {
    float2 offset = frac(texCoord * texSize - float2(0.5, 0.5));
    offset -= step(1.0, offset.x + offset.y);
    float4 c0 = TEX_OFFSET(tex, tSampler, texCoord, offset, texSize);
    float4 c1 = TEX_OFFSET(tex, tSampler, texCoord, float2(offset.x - sign(offset.x), offset.y), texSize);
    float4 c2 = TEX_OFFSET(tex, tSampler, texCoord, float2(offset.x, offset.y - sign(offset.y)), texSize);
    return c0 + abs(offset.x)*(c1-c0) + abs(offset.y)*(c2-c0);
}
PSInput VSMain(float4 position : POSITION, float2 uv0 : TEXCOORD0, float4 input1 : INPUT0, float4 input2 : INPUT1) {
    PSInput result;
    result.position = position;
    result.uv0 = uv0;    result.input1 = input1;
    result.input2 = input2;
    return result;
}
float4 PSMain(PSInput input) : SV_TARGET {
    float4 texVal0;
    if (textures[0].linear_filtering && filter == 2)
        texVal0 = tex2D3PointFilter(g_texture0, g_sampler0, input.uv0, float2(textures[0].width, textures[0].height));
    else
        texVal0 = g_texture0.Sample(g_sampler0, input.uv0);
    float4 texel = float4(texVal0.rgb * input.input1.rgb, input.input2.a);
    return texel;
}

*/

#define DECLARE_GFX_DXGI_FUNCTIONS
#include "gfx_window_dxgi.h"

#include "gfx_screen_config.h"

#define DEBUG_D3D 0

using namespace Microsoft::WRL; // For ComPtr

namespace {

// struct PerFrameCB {
//     uint32_t noise_frame;
//     float noise_scale_x;
//     float noise_scale_y;
//     uint32_t padding;
// };

// struct PerDrawCB {
//     struct Texture {
//         uint32_t width;
//         uint32_t height;
//         uint32_t linear_filtering;
//         uint32_t padding;
//     } textures[2];
//     uint32_t filter;
//     uint32_t padding[3];
// };

// struct LightmapCB {
//     float color[3];
//     float padding;
// };

// Shader struct items are aligned to 16-byte boundaries
// This is very important to take into account when updating arrays of uniforms
#define SHADER_ALIGN_BYTES  16

struct D3D11Uniforms {
    int ccs[CC_CHANNELS * CC_CYCLES];
    int ccf;

    float tex_properties[MAX_TEXTURES][3];

    float noise_frame_count;
    float light_map_color[3];

    int shader_flags;
    float shader_flag_values[SHADER_FLAG_MAX];
};

struct TextureData {
    ComPtr<ID3D11ShaderResourceView> resource_view;
    ComPtr<ID3D11SamplerState> sampler_state;
    uint32_t width;
    uint32_t height;
    uint8_t filter;
};

struct D3D11ShaderProgram {
    ComPtr<ID3D11VertexShader> vertex_shader;
    ComPtr<ID3D11PixelShader> pixel_shader;
    ComPtr<ID3D11InputLayout> input_layout;
    ComPtr<ID3D11BlendState> opaque_blend_state;
    ComPtr<ID3D11BlendState> alpha_blend_state;


    uint32_t uniform_locations[MAX_UNIFORMS];

    struct ColorCombiner cc;
    // uint64_t hash;
    // uint8_t num_inputs;
    // uint8_t num_floats;
    // bool used_textures[2];
    // bool used_lightmap;
};

static struct {
    HMODULE d3d11_module;
    PFN_D3D11_CREATE_DEVICE D3D11CreateDevice;

    HMODULE d3dcompiler_module;
    pD3DCompile D3DCompile;

    D3D_FEATURE_LEVEL feature_level;

    ComPtr<ID3D11Device> device;
    ComPtr<IDXGISwapChain1> swap_chain;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<ID3D11RenderTargetView> backbuffer_view;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view;
    ComPtr<ID3D11RasterizerState> rasterizer_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;
    ComPtr<ID3D11Buffer> vertex_buffer;
    ComPtr<ID3D11Buffer> cb;

#if DEBUG_D3D
    ComPtr<ID3D11Debug> debug;
#endif

    DXGI_SAMPLE_DESC sample_description;

    struct D3D11Uniforms cb_data;
    bool cb_dirty = true;
    uint32_t frame_count;

    // struct D3D11ShaderProgram shader_program_pool[CC_MAX_SHADERS];
    // uint8_t shader_program_pool_size;
    // uint8_t shader_program_pool_index;

    std::vector<struct TextureData> textures;
    int current_tile;
    uint32_t current_texture_ids[2];

    // Current state

    struct D3D11ShaderProgram shader_program;

    uint32_t current_width, current_height;

    int8_t depth_test;
    int8_t depth_mask;
    int8_t zmode_decal;

    // Previous states (to prevent setting states needlessly)

    ComPtr<ID3D11BlendState> last_blend_state = nullptr;
    ComPtr<ID3D11ShaderResourceView> last_resource_views[2] = { nullptr, nullptr };
    ComPtr<ID3D11SamplerState> last_sampler_states[2] = { nullptr, nullptr };
    int8_t last_depth_test = -1;
    int8_t last_depth_mask = -1;
    int8_t last_zmode_decal = -1;
    int8_t use_alpha = 0;
} d3d;



  //////////////
 // Uniforms //
//////////////

struct D3D11Uniform {
    const char *name;
    size_t offset;
    size_t count;
    size_t size;
    size_t stride;
};

#define uniform_11(UNIFORM) \
    offsetof(struct D3D11Uniforms, UNIFORM), 1, 1, tsizeof(struct D3D11Uniforms, UNIFORM)

#define uniform_1A(UNIFORM) \
    offsetof(struct D3D11Uniforms, UNIFORM), 1, countof(struct D3D11Uniforms, UNIFORM), tsizeof(struct D3D11Uniforms, UNIFORM)

#define uniform_A1(UNIFORM) \
    offsetof(struct D3D11Uniforms, UNIFORM), countof(struct D3D11Uniforms, UNIFORM), 1, tsizeof(struct D3D11Uniforms, UNIFORM[0])

#define uniform_AA(UNIFORM) \
    offsetof(struct D3D11Uniforms, UNIFORM), countof(struct D3D11Uniforms, UNIFORM), countof(struct D3D11Uniforms, UNIFORM[0]), tsizeof(struct D3D11Uniforms, UNIFORM[0])

static const struct D3D11Uniform D3D11_UNIFORMS[] = {
    [UNIFORM_CCS]                = { "uCCS",              uniform_A1(ccs)                },
    [UNIFORM_CCF]                = { "uCCF",              uniform_11(ccf)                },
    [UNIFORM_NOISE_FRAME_COUNT]  = { "uNoiseFrameCount",  uniform_11(noise_frame_count)  },
    [UNIFORM_LIGHT_MAP_COLOR]    = { "uLightMapColor",    uniform_1A(light_map_color)    },
    [UNIFORM_SHADER_FLAGS]       = { "uShaderFlags",      uniform_11(shader_flags)       },
    [UNIFORM_SHADER_FLAG_VALUES] = { "uShaderFlagValues", uniform_A1(shader_flag_values) },
    [UNIFORM_TEX_PROPERTIES]     = { "uTexProperties",    uniform_AA(tex_properties)     },
};

static void gfx_d3d11_locate_uniforms(struct D3D11ShaderProgram *prg, ID3DBlob *ps) {
    ComPtr<ID3D11ShaderReflection> reflection;
    ThrowIfFailed(D3DReflect(
        ps->GetBufferPointer(),
        ps->GetBufferSize(),
        IID_ID3D11ShaderReflection,
        reinterpret_cast<void **>(reflection.GetAddressOf())
    ));

    ID3D11ShaderReflectionConstantBuffer *buffer = reflection->GetConstantBufferByName("ShaderConstants");
    D3D11_SHADER_BUFFER_DESC buffer_desc;
    buffer->GetDesc(&buffer_desc);
    for (size_t i = 0; i < MAX_UNIFORMS; ++i) {
        const struct D3D11Uniform *uniform = &D3D11_UNIFORMS[i];
        ID3D11ShaderReflectionVariable *variable = buffer->GetVariableByName(uniform->name);
        if (!variable) {
            fprintf(stderr, "Uniform not found in pixel shader: %s\n", uniform->name);
            ThrowIfFailed(-1);
        }

        D3D11_SHADER_VARIABLE_DESC variable_desc;
        ThrowIfFailed(variable->GetDesc(&variable_desc));

        prg->uniform_locations[i] = variable_desc.StartOffset;

        printf("%-24s offset=%u size=%u\n",
               variable_desc.Name,
               variable_desc.StartOffset,
               variable_desc.Size);
    }
}


static void gfx_d3d11_set_shader_uniforms(struct D3D11ShaderProgram *prg, D3D11Uniforms *uniforms, void *dest) {

    static bool aaa = false;

    for (size_t i = 0; i < MAX_UNIFORMS; ++i) {
        const struct D3D11Uniform *uniform = &D3D11_UNIFORMS[i];
        for (size_t j = 0; j < uniform->count; ++j) {
            memcpy(
                (void *) (uintptr_t(dest)     + (prg->uniform_locations[i] + j * SHADER_ALIGN_BYTES)),
                (void *) (uintptr_t(uniforms) + (uniform->offset           + j * uniform->stride)),
                uniform->stride
            );

            if (!aaa) {
            printf("%-30s [%u] %4u (%4X) --<%2u>-> %4u (%4X)\n",
                uniform->name,
                j,
                (uniform->offset + j * uniform->stride),
                (uniform->offset + j * uniform->stride),
                uniform->stride,
                (prg->uniform_locations[i] + j * SHADER_ALIGN_BYTES),
                (prg->uniform_locations[i] + j * SHADER_ALIGN_BYTES)
            );


            }
        }
    }

    aaa=true;
}




static void create_render_target_views(bool is_resize) {
    DXGI_SWAP_CHAIN_DESC1 desc1;

    if (is_resize) {
        // Release previous stuff (if any)

        d3d.backbuffer_view.Reset();
        d3d.depth_stencil_view.Reset();

        // Resize swap chain buffers

        ThrowIfFailed(d3d.swap_chain->GetDesc1(&desc1));
        ThrowIfFailed(d3d.swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, desc1.Flags),
                      gfx_window_dxgi_get_h_wnd(), "Failed to resize IDXGISwapChain buffers.");
    }

    // Get new size

    ThrowIfFailed(d3d.swap_chain->GetDesc1(&desc1));

    // Create back buffer

    ComPtr<ID3D11Texture2D> backbuffer_texture;
    ThrowIfFailed(d3d.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *) backbuffer_texture.GetAddressOf()),
                  gfx_window_dxgi_get_h_wnd(), "Failed to get backbuffer from IDXGISwapChain.");

    ThrowIfFailed(d3d.device->CreateRenderTargetView(backbuffer_texture.Get(), nullptr, d3d.backbuffer_view.GetAddressOf()),
                  gfx_window_dxgi_get_h_wnd(), "Failed to create render target view.");

    // Create depth buffer

    D3D11_TEXTURE2D_DESC depth_stencil_texture_desc;
    ZeroMemory(&depth_stencil_texture_desc, sizeof(D3D11_TEXTURE2D_DESC));

    depth_stencil_texture_desc.Width = desc1.Width;
    depth_stencil_texture_desc.Height = desc1.Height;
    depth_stencil_texture_desc.MipLevels = 1;
    depth_stencil_texture_desc.ArraySize = 1;
    depth_stencil_texture_desc.Format = d3d.feature_level >= D3D_FEATURE_LEVEL_10_0 ?
                                        DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_stencil_texture_desc.SampleDesc = d3d.sample_description;
    depth_stencil_texture_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_stencil_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depth_stencil_texture_desc.CPUAccessFlags = 0;
    depth_stencil_texture_desc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> depth_stencil_texture;
    ThrowIfFailed(d3d.device->CreateTexture2D(&depth_stencil_texture_desc, nullptr, depth_stencil_texture.GetAddressOf()));
    ThrowIfFailed(d3d.device->CreateDepthStencilView(depth_stencil_texture.Get(), nullptr, d3d.depth_stencil_view.GetAddressOf()));

    // Save resolution

    d3d.current_width = desc1.Width;
    d3d.current_height = desc1.Height;
}




static void gfx_d3d11_load_ubershader(void) {

    // Create main vertex buffer

    D3D11_BUFFER_DESC vertex_buffer_desc;
    ZeroMemory(&vertex_buffer_desc, sizeof(D3D11_BUFFER_DESC));

    vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertex_buffer_desc.ByteWidth = sizeof(VertexAttribs) * MAX_BUFFERED * 3; // Same as buf_vbo size in gfx_pc
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertex_buffer_desc.MiscFlags = 0;

    ThrowIfFailed(d3d.device->CreateBuffer(&vertex_buffer_desc, nullptr, d3d.vertex_buffer.GetAddressOf()),
                  gfx_window_dxgi_get_h_wnd(), "Failed to create vertex buffer.");

    D3D11_BUFFER_DESC constant_buffer_desc;
    ZeroMemory(&constant_buffer_desc, sizeof(D3D11_BUFFER_DESC));
    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.ByteWidth = 240;//sizeof(D3D11Uniforms); TODO
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;

    ThrowIfFailed(d3d.device->CreateBuffer(&constant_buffer_desc, nullptr, d3d.cb.GetAddressOf()),
                  gfx_window_dxgi_get_h_wnd(), "Failed to create shader constant buffer.");

    d3d.context->PSSetConstantBuffers(0, 1, d3d.cb.GetAddressOf());

    const char *buf = D3D11_UBERSHADER;
    size_t len = sizeof(D3D11_UBERSHADER) - 1;

    ComPtr<ID3DBlob> vs, ps;
    ComPtr<ID3DBlob> error_blob;

#if DEBUG_D3D
    UINT compile_flags = D3DCOMPILE_DEBUG;
#else
    UINT compile_flags = D3DCOMPILE_OPTIMIZATION_LEVEL2;
#endif

    HRESULT hr = d3d.D3DCompile(buf, len, nullptr, nullptr, nullptr, "VSMain", "vs_4_0", compile_flags, 0, vs.GetAddressOf(), error_blob.GetAddressOf());

    if (FAILED(hr)) {
        fprintf(stderr, "Error: 0x%08X\n%s", hr, (const char *) error_blob->GetBufferPointer());
        MessageBox(gfx_window_dxgi_get_h_wnd(), (char *) error_blob->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
        throw hr;
    }

    hr = d3d.D3DCompile(buf, len, nullptr, nullptr, nullptr, "PSMain", "ps_4_0", compile_flags, 0, ps.GetAddressOf(), error_blob.GetAddressOf());

    if (FAILED(hr)) {
        fprintf(stderr, "Error: 0x%08X\n%s", hr, (const char *) error_blob->GetBufferPointer());
        MessageBox(gfx_window_dxgi_get_h_wnd(), (char *) error_blob->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
        throw hr;
    }

    printf("Compilation done!\n");

    struct D3D11ShaderProgram *prg = &d3d.shader_program;
    // d3d.shader_program_pool[d3d.shader_program_pool_index];
    // d3d.shader_program_pool_index = (d3d.shader_program_pool_index + 1) % CC_MAX_SHADERS;
    // if (d3d.shader_program_pool_size < CC_MAX_SHADERS) { d3d.shader_program_pool_size++; }

    ThrowIfFailed(d3d.device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, prg->vertex_shader.GetAddressOf()));
    ThrowIfFailed(d3d.device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, prg->pixel_shader.GetAddressOf()));

    // Input Layout

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexAttribs, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexAttribs, tex_coords[0]), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexAttribs, tex_coords[1]), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "INPUT", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(VertexAttribs, inputs[0]), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "INPUT", 1, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(VertexAttribs, inputs[1]), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "INPUT", 2, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(VertexAttribs, inputs[2]), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "INPUT", 3, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(VertexAttribs, inputs[3]), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "INPUT", 4, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(VertexAttribs, inputs[4]), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "INPUT", 5, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(VertexAttribs, inputs[5]), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "INPUT", 6, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(VertexAttribs, inputs[6]), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "FOG", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(VertexAttribs, fog), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "LIGHTMAP", 0, DXGI_FORMAT_R16G16_UNORM, 0, offsetof(VertexAttribs, light_map), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    ThrowIfFailed(d3d.device->CreateInputLayout(ied, ARRAYSIZE(ied), vs->GetBufferPointer(), vs->GetBufferSize(), prg->input_layout.GetAddressOf()));

    // Blend state

    D3D11_BLEND_DESC blend_desc;
    ZeroMemory(&blend_desc, sizeof(D3D11_BLEND_DESC));

    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ThrowIfFailed(d3d.device->CreateBlendState(&blend_desc, prg->opaque_blend_state.GetAddressOf()));

    blend_desc.RenderTarget[0].BlendEnable = true;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    ThrowIfFailed(d3d.device->CreateBlendState(&blend_desc, prg->alpha_blend_state.GetAddressOf()));

    gfx_d3d11_locate_uniforms(prg, ps.Get());
}



static void gfx_d3d11_init(void) {
    // Load d3d11.dll
    d3d.d3d11_module = LoadLibraryW(L"d3d11.dll");
    if (d3d.d3d11_module == nullptr) {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), gfx_window_dxgi_get_h_wnd(), "d3d11.dll could not be loaded");
    }
    d3d.D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(d3d.d3d11_module, "D3D11CreateDevice");

    // Load D3DCompiler_47.dll or D3DCompiler_43.dll
    d3d.d3dcompiler_module = LoadLibraryW(L"D3DCompiler_47.dll");
    if (d3d.d3dcompiler_module == nullptr) {
        d3d.d3dcompiler_module = LoadLibraryW(L"D3DCompiler_43.dll");
        if (d3d.d3dcompiler_module == nullptr) {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), gfx_window_dxgi_get_h_wnd(), "D3DCompiler_47.dll or D3DCompiler_43.dll could not be loaded");
        }
    }
    d3d.D3DCompile = (pD3DCompile)GetProcAddress(d3d.d3dcompiler_module, "D3DCompile");

    // Create D3D11 device

    gfx_window_dxgi_create_factory_and_device(DEBUG_D3D, 11, [](IDXGIAdapter1 *adapter, bool test_only) {
#if DEBUG_D3D
        UINT device_creation_flags = D3D11_CREATE_DEVICE_DEBUG;
#else
        UINT device_creation_flags = 0;
#endif
        D3D_FEATURE_LEVEL FeatureLevels[] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        HRESULT res = d3d.D3D11CreateDevice(
            adapter,
            D3D_DRIVER_TYPE_UNKNOWN, // since we use a specific adapter
            nullptr,
            device_creation_flags,
            FeatureLevels,
            ARRAYSIZE(FeatureLevels),
            D3D11_SDK_VERSION,
            test_only ? nullptr : d3d.device.GetAddressOf(),
            &d3d.feature_level,
            test_only ? nullptr : d3d.context.GetAddressOf());

        if (test_only) {
            return SUCCEEDED(res);
        } else {
            ThrowIfFailed(res, gfx_window_dxgi_get_h_wnd(), "Failed to create D3D11 device.");
            return true;
        }
    });

    // Sample description to be used in back buffer and depth buffer

    d3d.sample_description.Count = 1;
    d3d.sample_description.Quality = 0;

    // Create the swap chain
    d3d.swap_chain = gfx_window_dxgi_create_swap_chain(d3d.device.Get());

    // Create D3D Debug device if in debug mode

#if DEBUG_D3D
    ThrowIfFailed(d3d.device->QueryInterface(__uuidof(ID3D11Debug), (void **) d3d.debug.GetAddressOf()),
                  gfx_window_dxgi_get_h_wnd(), "Failed to get ID3D11Debug device.");
#endif

    // Create views

    create_render_target_views(false);

    gfx_d3d11_load_ubershader();
}


static bool gfx_d3d11_z_is_from_0_to_1(void) {
    return true;
}

static bool gfx_d3d11_update_cc(struct ColorCombiner *cc) {
    // if (memcmp(&d3d.shader_program.cc, cc, sizeof(*cc)) == 0) {
    //     return false;
    // }
    d3d.shader_program.cc = *cc;
    return true;
}

static void gfx_d3d11_update_uniforms(void) {
    struct ColorCombiner *cc = &d3d.shader_program.cc;
    for (int i = 0; i < CC_CHANNELS * CC_CYCLES; ++i) {
        uint32_t packed_ccs =
            ((uint32_t)cc->ccs[i * CC_COMPONENTS + 0] << 24) |
            ((uint32_t)cc->ccs[i * CC_COMPONENTS + 1] << 16) |
            ((uint32_t)cc->ccs[i * CC_COMPONENTS + 2] << 8) |
            ((uint32_t)cc->ccs[i * CC_COMPONENTS + 3] << 0);
        d3d.cb_data.ccs[i] = (int32_t)packed_ccs;
    }

    d3d.cb_data.ccf = 0;
    d3d.cb_data.ccf |= cc->use_noise << CCF_NOISE;
    d3d.cb_data.ccf |= cc->use_alpha << CCF_USE_ALPHA;
    d3d.cb_data.ccf |= cc->use_fog << CCF_USE_FOG;
    d3d.cb_data.ccf |= cc->texture_edge << CCF_TEX_EDGE;
    d3d.cb_data.ccf |= cc->use_dither << CCF_USE_DITHER;
    d3d.cb_data.ccf |= cc->use_2cycle << CCF_USE_2CYCLE;
    d3d.cb_data.ccf |= cc->light_map << CCF_LIGHT_MAP;
    d3d.cb_data.ccf |= cc->world_geometry << CCF_WORLD_GEOMETRY;
    d3d.cb_data.ccf |= uint32_t(cc->used_textures) << CCF_USED_TEXTURE;

    if ((cc->use_alpha && cc->use_dither) || cc->use_noise) {
        d3d.cb_data.noise_frame_count = (float)d3d.frame_count;
    }
    if (cc->light_map) {
        d3d.cb_data.light_map_color[0] = gVertexColor[0] / 255.0f;
        d3d.cb_data.light_map_color[1] = gVertexColor[1] / 255.0f;
        d3d.cb_data.light_map_color[2] = gVertexColor[2] / 255.0f;
    }
    if (cc->world_geometry) {
        d3d.cb_data.shader_flags = 0;
        for (int i = 0; i < SHADER_FLAG_MAX; ++i) {
            d3d.cb_data.shader_flags |= (int)gShaderFlags[i] << i;
            d3d.cb_data.shader_flag_values[i] = gShaderFlagValues[i];
        }
    }
    d3d.cb_dirty = true;
}

static uint32_t gfx_d3d11_new_texture(void) {
    d3d.textures.resize(d3d.textures.size() + 1);
    return (uint32_t)(d3d.textures.size() - 1);
}

static void gfx_d3d11_select_texture(int tile, uint32_t texture_id) {
    d3d.current_tile = tile;
    d3d.current_texture_ids[tile] = texture_id;
    TextureData &texture_data = d3d.textures[texture_id];
    d3d.cb_data.tex_properties[tile][0] = (float)texture_data.width;
    d3d.cb_data.tex_properties[tile][1] = (float)texture_data.height;
    d3d.cb_data.tex_properties[tile][2] = (float)texture_data.filter;
    d3d.cb_dirty = true;
}

static D3D11_TEXTURE_ADDRESS_MODE gfx_cm_to_d3d11(uint32_t val) {
    if (val & G_TX_CLAMP) {
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    }
    return (val & G_TX_MIRROR) ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
}

static void gfx_d3d11_upload_texture(const uint8_t *rgba32_buf, int width, int height) {
    // Create texture

    D3D11_TEXTURE2D_DESC texture_desc;
    ZeroMemory(&texture_desc, sizeof(D3D11_TEXTURE2D_DESC));

    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_desc.CPUAccessFlags = 0;
    texture_desc.MiscFlags = 0; // D3D11_RESOURCE_MISC_GENERATE_MIPS ?
    texture_desc.ArraySize = 1;
    texture_desc.MipLevels = 1;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;

    D3D11_SUBRESOURCE_DATA resource_data;
    resource_data.pSysMem = rgba32_buf;
    resource_data.SysMemPitch = width * 4;
    resource_data.SysMemSlicePitch = resource_data.SysMemPitch * height;

    ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed(d3d.device->CreateTexture2D(&texture_desc, &resource_data, texture.GetAddressOf()));

    // Create shader resource view from texture

    D3D11_SHADER_RESOURCE_VIEW_DESC resource_view_desc;
    ZeroMemory(&resource_view_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    resource_view_desc.Format = texture_desc.Format;
    resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resource_view_desc.Texture2D.MostDetailedMip = 0;
    resource_view_desc.Texture2D.MipLevels = 1;//-1;

    TextureData *texture_data = &d3d.textures[d3d.current_texture_ids[d3d.current_tile]];
    texture_data->width = width;
    texture_data->height = height;

    if (texture_data->resource_view.Get() != nullptr) {
        // Free the previous texture in this slot
        texture_data->resource_view.Reset();
    }

    ThrowIfFailed(d3d.device->CreateShaderResourceView(texture.Get(), &resource_view_desc, texture_data->resource_view.GetAddressOf()));
}

static void gfx_d3d11_set_sampler_parameters(int tile, uint8_t filter, uint32_t cms, uint32_t cmt) {
    D3D11_SAMPLER_DESC sampler_desc;
    ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));

    sampler_desc.Filter = filter != 0 ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampler_desc.AddressU = gfx_cm_to_d3d11(cms);
    sampler_desc.AddressV = gfx_cm_to_d3d11(cmt);
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

    TextureData *texture_data = &d3d.textures[d3d.current_texture_ids[tile]];
    texture_data->filter = filter;
    d3d.cb_data.tex_properties[tile][2] = (float)filter;
    d3d.cb_dirty = true;

    // This function is called twice per texture, the first one only to set default values.
    // Maybe that could be skipped? Anyway, make sure to release the first default sampler
    // state before setting the actual one.
    texture_data->sampler_state.Reset();

    ThrowIfFailed(d3d.device->CreateSamplerState(&sampler_desc, texture_data->sampler_state.GetAddressOf()));
}

static void gfx_d3d11_set_depth_test(bool depth_test) {
    d3d.depth_test = depth_test;
}

static void gfx_d3d11_set_depth_mask(bool depth_mask) {
    d3d.depth_mask = depth_mask;
}

static void gfx_d3d11_set_zmode_decal(bool zmode_decal) {
    d3d.zmode_decal = zmode_decal;
}

static void gfx_d3d11_set_viewport(int x, int y, int width, int height) {
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = x;
    viewport.TopLeftY = d3d.current_height - y - height;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    d3d.context->RSSetViewports(1, &viewport);
}

static void gfx_d3d11_set_scissor(int x, int y, int width, int height) {
    D3D11_RECT rect;
    rect.left = x;
    rect.top = d3d.current_height - y - height;
    rect.right = x + width;
    rect.bottom = d3d.current_height - y;

    d3d.context->RSSetScissorRects(1, &rect);
}

static void gfx_d3d11_set_use_alpha(bool use_alpha) {
    d3d.use_alpha = use_alpha;
}

static void gfx_d3d11_draw_triangles(struct VertexAttribs *buf_vbo, size_t buf_vbo_len, size_t buf_vbo_num_tris) {

    if (d3d.last_depth_test != d3d.depth_test || d3d.last_depth_mask != d3d.depth_mask) {
        d3d.last_depth_test = d3d.depth_test;
        d3d.last_depth_mask = d3d.depth_mask;

        d3d.depth_stencil_state.Reset();

        D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
        ZeroMemory(&depth_stencil_desc, sizeof(D3D11_DEPTH_STENCIL_DESC));

        depth_stencil_desc.DepthEnable = d3d.depth_test;
        depth_stencil_desc.DepthWriteMask = d3d.depth_mask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depth_stencil_desc.StencilEnable = false;

        ThrowIfFailed(d3d.device->CreateDepthStencilState(&depth_stencil_desc, d3d.depth_stencil_state.GetAddressOf()));
        d3d.context->OMSetDepthStencilState(d3d.depth_stencil_state.Get(), 0);
    }

    if (d3d.last_zmode_decal != d3d.zmode_decal) {
        d3d.last_zmode_decal = d3d.zmode_decal;

        d3d.rasterizer_state.Reset();

        D3D11_RASTERIZER_DESC rasterizer_desc;
        ZeroMemory(&rasterizer_desc, sizeof(D3D11_RASTERIZER_DESC));

        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.FrontCounterClockwise = true;
        rasterizer_desc.DepthBias = 0;
        rasterizer_desc.SlopeScaledDepthBias = d3d.zmode_decal ? -2.0f : 0.0f;
        rasterizer_desc.DepthBiasClamp = 0.0f;
        rasterizer_desc.DepthClipEnable = true;
        rasterizer_desc.ScissorEnable = true;
        rasterizer_desc.MultisampleEnable = false;
        rasterizer_desc.AntialiasedLineEnable = false;

        ThrowIfFailed(d3d.device->CreateRasterizerState(&rasterizer_desc, d3d.rasterizer_state.GetAddressOf()));
        d3d.context->RSSetState(d3d.rasterizer_state.Get());
    }

    for (int32_t i = 0; i < MAX_TEXTURES; i++) {
        if (d3d.shader_program.cc.used_textures & USED_TEXTURE(i)) {
            TextureData &texture_data = d3d.textures[d3d.current_texture_ids[i]];
            bool resource_changed = d3d.last_resource_views[i].Get() != texture_data.resource_view.Get();
            bool sampler_changed = d3d.last_sampler_states[i].Get() != texture_data.sampler_state.Get();

            // if (resource_changed) {
                d3d.last_resource_views[i] = texture_data.resource_view.Get();
                d3d.context->PSSetShaderResources(i, 1, texture_data.resource_view.GetAddressOf());
            // }

            // if (sampler_changed) {
                d3d.last_sampler_states[i] = texture_data.sampler_state.Get();
                d3d.context->PSSetSamplers(i, 1, texture_data.sampler_state.GetAddressOf());
            // }

            d3d.cb_data.tex_properties[i][0] = (float)texture_data.width;
            d3d.cb_data.tex_properties[i][1] = (float)texture_data.height;
            d3d.cb_data.tex_properties[i][2] = (float)texture_data.filter;
            d3d.cb_dirty |= resource_changed || sampler_changed;
        }
    }

    // Set vertex buffer data

    D3D11_MAPPED_SUBRESOURCE ms;
    ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
    ThrowIfFailed(d3d.context->Map(d3d.vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms));
    memcpy(ms.pData, buf_vbo, buf_vbo_len * sizeof(VertexAttribs));
    d3d.context->Unmap(d3d.vertex_buffer.Get(), 0);

    uint32_t stride = sizeof(VertexAttribs);
    uint32_t offset = 0;
    d3d.context->IASetVertexBuffers(0, 1, d3d.vertex_buffer.GetAddressOf(), &stride, &offset);

    struct D3D11ShaderProgram &program = d3d.shader_program;
    d3d.context->IASetInputLayout(program.input_layout.Get());
    d3d.context->VSSetShader(program.vertex_shader.Get(), 0, 0);
    d3d.context->PSSetShader(program.pixel_shader.Get(), 0, 0);

    ID3D11BlendState *blend_state = d3d.use_alpha ? program.alpha_blend_state.Get() : program.opaque_blend_state.Get();
    // if (d3d.last_blend_state.Get() != blend_state) {
        d3d.last_blend_state = blend_state;
        d3d.context->OMSetBlendState(blend_state, 0, 0xFFFFFFFF);
    // }

    // if (d3d.cb_dirty) {
        ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
        ThrowIfFailed(d3d.context->Map(d3d.cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms));
        memcpy(ms.pData, &d3d.cb_data, sizeof(D3D11Uniforms)); //TODO

        gfx_d3d11_set_shader_uniforms(&d3d.shader_program, &d3d.cb_data, ms.pData);


        d3d.context->Unmap(d3d.cb.Get(), 0);
        d3d.cb_dirty = false;
    // }

    d3d.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d.context->Draw(buf_vbo_num_tris * 3, 0);
}

static void gfx_d3d11_on_resize(void) {
    create_render_target_views(true);
}

static void gfx_d3d11_start_frame(void) {
    // Set render targets

    d3d.context->OMSetRenderTargets(1, d3d.backbuffer_view.GetAddressOf(), d3d.depth_stencil_view.Get());

    // Clear render targets

    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    d3d.context->ClearRenderTargetView(d3d.backbuffer_view.Get(), clearColor);
    d3d.context->ClearDepthStencilView(d3d.depth_stencil_view.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    d3d.frame_count = (d3d.frame_count + 1) % 151;
    if ((d3d.shader_program.cc.use_alpha && d3d.shader_program.cc.use_dither) || d3d.shader_program.cc.use_noise) {
        d3d.cb_data.noise_frame_count = (float)d3d.frame_count;
        d3d.cb_dirty = true;
    }
}

static void gfx_d3d11_end_frame(void) {
}

static const char* gfx_d3d11_get_name(void) {
    return "DirectX 11";
}

static void gfx_d3d11_finish_render(void) {
}

} // namespace

struct GfxRenderingAPI gfx_direct3d11_api = {
    .z_is_from_0_to_1 = gfx_d3d11_z_is_from_0_to_1,
    .update_cc = gfx_d3d11_update_cc,
    .update_uniforms = gfx_d3d11_update_uniforms,
    .new_texture = gfx_d3d11_new_texture,
    .select_texture = gfx_d3d11_select_texture,
    .upload_texture = gfx_d3d11_upload_texture,
    .set_sampler_parameters = gfx_d3d11_set_sampler_parameters,
    .set_depth_test = gfx_d3d11_set_depth_test,
    .set_depth_mask = gfx_d3d11_set_depth_mask,
    .set_zmode_decal = gfx_d3d11_set_zmode_decal,
    .set_viewport = gfx_d3d11_set_viewport,
    .set_scissor = gfx_d3d11_set_scissor,
    .set_use_alpha = gfx_d3d11_set_use_alpha,
    .draw_triangles = gfx_d3d11_draw_triangles,
    .init = gfx_d3d11_init,
    .on_resize = gfx_d3d11_on_resize,
    .start_frame = gfx_d3d11_start_frame,
    .end_frame = gfx_d3d11_end_frame,
    .finish_render = gfx_d3d11_finish_render,
    .get_name = gfx_d3d11_get_name,
    .shutdown = NULL,
};

#endif
