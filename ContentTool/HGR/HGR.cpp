#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>

#include "HGR.h"
#include "../ToolCommon.h"

namespace tools::hgr {
	namespace {
        constexpr u32 su16{ sizeof(u16) }; // 2 bytes for reading
        constexpr u32 su32{ sizeof(u32) }; // 4 bytes for reading

        constexpr bool is_big_endian = (std::endian::native == std::endian::big);

        bool check_signature(const u8*& at) {
            // Check Signature
            char str[5]; memcpy(str, at, 5);
            // if (str != "hgrf") return false; // Fails to check RN
            at += 5;

            return true;
        }

        bool check_id(const u8*& at, hgr_info& info) {
            u32 id{ ++info.check_id };
            memcpy(&(info.check_id), at, su32); at += su32; info.check_id = swap_endian<u32>(info.check_id);
            assert(info.check_id == id, "Check ID Failed");
            return true;
        }

        bool read_buffer(const u8*& at, hgr_info& info) {
            memcpy(&(info.m_ver), at, 1); at += 1;
            if (info.m_ver > 190) memcpy(&(info.m_exportedVer), at, su32); at += su32; // Currently reading little Endian
            memcpy(&(info.m_dataFlags), at, su16); at += su16;
            if (info.m_ver > 180) memcpy(&(info.m_platformID), at, su16); at += su16;

            return true;
        }

        bool read_buffer(const u8*& at, scene_param_info& info) {
            memcpy(&(info.fogType), at, 1); at += 1;
            memcpy(&(info.fogStart), at, su32); at += su32;
            memcpy(&(info.fogEnd), at, su32); at += su32;

            memcpy(&(info.fogColour), at, su32 * 3); at += su32 * 3;

            return true;
        }

        bool read_buffer(const u8*& at, texture_info*& info, u32& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].name.assign(at, at + size); at += size;
                memcpy(&(info[i].type), at, su32); at += su32;
            }
            return true;
        }

        bool read_buffer(const u8*& at, texParam*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].param_type.assign(at, at + size); at += size; // param Type

                memcpy(&(info[i].texIndex), at, su16); at += su16; info[i].texIndex = swap_endian<u16>(info[i].texIndex);
            }
            return true;
        }

        bool read_buffer(const u8*& at, vec4Param*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].param_type.assign(at, at + size); at += size; // param Type

                memcpy(&(info[i].value[0]), at, su32); at += su32; info[i].value[0] = swap_endian<f32>(info[i].value[0]);
                memcpy(&(info[i].value[1]), at, su32); at += su32; info[i].value[1] = swap_endian<f32>(info[i].value[1]);
                memcpy(&(info[i].value[2]), at, su32); at += su32; info[i].value[2] = swap_endian<f32>(info[i].value[2]);
                memcpy(&(info[i].value[3]), at, su32); at += su32; info[i].value[3] = swap_endian<f32>(info[i].value[3]);
            }
            return true;
        }

        bool read_buffer(const u8*& at, floatParam*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].param_type.assign(at, at + size); at += size; // param Type

                memcpy(&(info[i].value), at, su32); at += su32; info[i].value = swap_endian<f32>(info[i].value);
            }
            return true;
        }

        bool read_buffer(const u8*& at, material_info*& info, u32& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].name.assign(at, at + size); at += size; // name
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].shaderName.assign(at, at + size); at += size; // shaderName
                memcpy(&(info[i].lightmap_info), at, su32); at += su32; info[i].lightmap_info = swap_endian<s32>(info[i].lightmap_info);

                memcpy(&(info[i].texParamCount), at, 1); at += 1; info[i].texParamCount = swap_endian<u8>(info[i].texParamCount);
                info[i].TexParams = new texParam[info[i].texParamCount];
                read_buffer(at, info[i].TexParams, info[i].texParamCount);

                memcpy(&(info[i].vec4ParamCount), at, 1); at += 1; info[i].vec4ParamCount = swap_endian<u8>(info[i].vec4ParamCount);
                info[i].Vec4Params = new vec4Param[info[i].vec4ParamCount];
                read_buffer(at, info[i].Vec4Params, info[i].vec4ParamCount);

                memcpy(&(info[i].floatParamCount), at, 1); at += 1; info[i].floatParamCount = swap_endian<u8>(info[i].floatParamCount);
                info[i].FloatParams = new floatParam[info[i].floatParamCount];
                read_buffer(at, info[i].FloatParams, info[i].floatParamCount);
            }
            return true;
        }

        bool read_file(std::filesystem::path path, std::unique_ptr<u8[]>& data, u64& size) {
            if (!std::filesystem::exists(path)) return false;
            assert(true);

            size = std::filesystem::file_size(path);
            if (!size) return false;
            data = std::make_unique<u8[]>(size);
            std::ifstream file{ path, std::ios::in | std::ios::binary }; // Create a filestream
            if (!file || !file.read((char*)data.get(), size)) {
                file.close();
                return false;
            }

            file.close();
            return true;
        }
	} // Anonymous Namespace

    TOOL_INTERFACE bool StoreData(const char* path) {
        std::unique_ptr<u8[]> buffer{};
        u64 size{ 0 };
        if (!read_file(path, buffer, size)) return false;
        assert(buffer.get());
        const u8* at{ buffer.get() };

        if (!check_signature(at)) return false;

        hgr_info* header = new hgr_info();
        //std::shared_ptr<hgr::hgr_info> header{}; // I don't know why smart pointer is causing errors
        read_buffer(at, *header);
        scene_param_info* sceneParams = new scene_param_info();
        read_buffer(at, *sceneParams);
        memcpy(&(header->check_id), at, su32); at += su32;
        header->check_id = swap_endian<u32>(header->check_id);

        entity_info entityInfo{};
        memcpy(&(entityInfo.Texture_Count), at, su32); at += su32; 
        entityInfo.Texture_Count = swap_endian<u32>(entityInfo.Texture_Count);
        texture_info* Textures = new texture_info[entityInfo.Texture_Count];
        read_buffer(at, Textures, entityInfo.Texture_Count);

        memcpy(&(entityInfo.Material_Count), at, su32); at += su32;
        entityInfo.Material_Count = swap_endian<u32>(entityInfo.Material_Count);
        material_info* Materials = new material_info[entityInfo.Material_Count];
        read_buffer(at, Materials, entityInfo.Material_Count);

        check_id(at, *header);

        memcpy(&(entityInfo.Primitive_Count), at, su32); at += su32;

        check_id(at, *header);

        memcpy(&(entityInfo.Mesh_Count), at, su32); at += su32;

        //check_id(at, *header);

        memcpy(&(entityInfo.Camera_Count), at, su32); at += su32;

        //check_id(at, *header);

        memcpy(&(entityInfo.Light_Count), at, su32); at += su32;

        //check_id(at, *header);

        memcpy(&(entityInfo.Dummy_Count), at, su32); at += su32;

        //check_id(at, *header);

        memcpy(&(entityInfo.Shape_Count), at, su32); at += su32;

        //check_id(at, *header);

        memcpy(&(entityInfo.OtherNodes_Count), at, su32); at += su32;

        //check_id(at, *header);

        memcpy(&(entityInfo.TransformAnimation_Count), at, su32); at += su32;

        //check_id(at, *header);

        memcpy(&(entityInfo.UserProperties_Count), at, su32); at += su32;

        //std::unique_ptr<hgr::mesh> Mesh{};
        mesh* Mesh = new mesh();
        Mesh->name = "James";

        //delete Mesh; // to avoid memory leaks
        return true;
    }
}