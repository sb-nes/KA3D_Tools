#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>

#include "HGR.h"
#include "../ToolCommon.h"
#include "Entity.h"

#define MAX_TEXCOORDS = 4 

namespace tools::hgr {
	namespace {
        constexpr u32 su16{ sizeof(u16) }; // 2 bytes for reading
        constexpr u32 su32{ sizeof(u32) }; // 4 bytes for reading

        constexpr bool is_big_endian = (std::endian::native == std::endian::big);

        bool check_signature(const u8*& at) {
            // Check Signature
            char magic[4]; memcpy(magic, at, 4);
            int z = memcmp(magic, "hgrfi", 4);
            //if (str != "hgrf") return false; // Fails to check RN
            at += 5;

            return true;
        }

        bool check_id(const u8*& at, hgr_info& info) {
            u32 id{ ++info.check_id };
            memcpy(&(info.check_id), at, su32); at += su32; info.check_id = swap_endian<u32>(info.check_id);
            assert(info.check_id == id && "Check ID Failed");
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
            for (u32 i{ 0 };i < count;++i) {
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

                memcpy(&(info[i].value), at, su32 * 4); at += su32 * 4; 
                info[i].value[0] = swap_endian<f32>(info[i].value[0]);
                info[i].value[1] = swap_endian<f32>(info[i].value[1]);
                info[i].value[2] = swap_endian<f32>(info[i].value[2]);
                info[i].value[3] = swap_endian<f32>(info[i].value[3]);
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
            for (u32 i{ 0 };i < count;++i) {
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

        bool read_buffer(const u8*& at, vertFormat*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].type.assign(at, at + size); at += size; // type with "DT_" prefix
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].format.assign(at, at + size); at += size; // format without "DF_" prefix
                info[i].format = "DF_" + info[i].format;
            }
            return true;
        }

        bool read_buffer(const u8*& at, vertArray*& info, u8& count, u32 verts, vertFormat* formats) {
            u32 size{ 0 };
            u32 length{ 0 };
            for (int i{ 0 };i < count;++i) {
                switch (dfMap[formats[i].format]) {
                case DF_S_16:
                    size = verts;
                    length = su16;
                    break;

                case DF_V2_16:
                    size = verts * 2;
                    length = su16;
                    break;

                case DF_V3_16:
                    size = verts * 3;
                    length = su16;
                    break;

                case DF_V4_16:
                    size = verts * 4;
                    length = su16;
                    break;

                default:
                    assert(false && "Unimplemented type: Go into Debugger Mode to check the format!");
                    return false;
                }

                memcpy(&(info[i].scale), at, su32); at += su32;
                info[i].scale = swap_endian<f32>(info[i].scale);
                memcpy(&(info[i].bias), at, su32 * 3); at += su32 * 3;

                for (int j{ 0 };j < 3;++j) {
                    info[i].bias[j] = swap_endian<f32>(info[i].bias[j]);
                }

                info[i].value = new s16[size];
                info[i].actualValue = new f32[size];

                memcpy(info[i].value, at, size * length); at += size * length;
                for (u32 j{ 0 };j < size;++j) {
                    info[i].value[j] = swap_endian<s16>(info[i].value[j]); // is it supposed to be little or big endian?
                    info[i].actualValue[j] = (info[i].value[j] * info[i].scale) + info[i].bias[j % (size / verts)];
                }

                info[i].size = size;
            }
            return true;
        }

        bool read_buffer(const u8*& at, primitive_info*& info, u32& count) {
            for (u32 i{ 0 };i < count;++i) {
                memcpy(&(info[i].verts), at, su32); at += su32; info[i].verts = swap_endian<u32>(info[i].verts);
                memcpy(&(info[i].indices), at, su32); at += su32; info[i].indices = swap_endian<u32>(info[i].indices);

                memcpy(&(info[i].formatCount), at, 1); at += 1;
                info[i].formats = new vertFormat[info[i].formatCount];
                info[i].vArray = new vertArray[info[i].formatCount];
                read_buffer(at, info[i].formats, info[i].formatCount);

                memcpy(&(info[i].matIndex), at, su16); at += su16; info[i].matIndex = swap_endian<u16>(info[i].matIndex);
                memcpy(&(info[i].primitiveType), at, su16); at += su16; info[i].primitiveType = swap_endian<u16>(info[i].primitiveType);

                read_buffer(at, info[i].vArray, info[i].formatCount, info[i].verts, info[i].formats);

                info[i].indexData = new u16[info[i].indices];
                memcpy(info[i].indexData, at, su16 * info[i].indices); at += su16 * info[i].indices;
                for (u32 j{ 0 };j < info[i].indices;++j) {
                    info[i].indexData[j] = swap_endian<u16>(info[i].indexData[j]);
                }
                
                memcpy(&(info[i].usedBoneCount), at, 1); at += 1; info[i].usedBoneCount = swap_endian<u8>(info[i].usedBoneCount);

                info[i].usedBones = new u8[info[i].usedBoneCount];
                memcpy(info[i].usedBones, at, info[i].usedBoneCount); at += info[i].usedBoneCount;
            }
            return true;
        }

        bool read_buffer(const u8*& at, node& info) {
            u16 size{ 0 }; u32 i{ 0 };
            memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
            info.name.assign(at, at + size); at += size; // name

            // model tm
            for (i = 0;i < 3;++i) {
                memcpy(&info.modeltm.x[i], at, su32); at += su32; 
                memcpy(&info.modeltm.y[i], at, su32); at += su32;
                memcpy(&info.modeltm.z[i], at, su32); at += su32;
                memcpy(&info.modeltm.w[i], at, su32); at += su32;
                info.modeltm.x[i] = swap_endian<f32>(info.modeltm.x[i]);
                info.modeltm.y[i] = swap_endian<f32>(info.modeltm.y[i]);
                info.modeltm.z[i] = swap_endian<f32>(info.modeltm.z[i]);
                info.modeltm.w[i] = swap_endian<f32>(info.modeltm.w[i]);
            }

            memcpy(&(info.nodeFlags), at, su32); at += su32;
            info.nodeFlags = swap_endian<u32>(info.nodeFlags);

            memcpy(&(info.id), at, su32); at += su32;
            info.id = swap_endian<u32>(info.id);

            memcpy(&(info.parentIndex), at, su32); at += su32;
            info.parentIndex = swap_endian<u32>(info.parentIndex); // u32_invalid_id = -1 -> iron-blooded orphan

            return true;
        }

        bool read_buffer(const u8*& at, meshbone& info) {
            u32 i{ 0 };

            memcpy(&(info.boneNodeIndex), at, su32); at += su32;
            info.boneNodeIndex = swap_endian<u32>(info.boneNodeIndex);

            // invresttm
            for (i = 0;i < 3;++i) {
                memcpy(&info.invresttm.x[i], at, su32); at += su32;
                memcpy(&info.invresttm.y[i], at, su32); at += su32;
                memcpy(&info.invresttm.z[i], at, su32); at += su32;
                memcpy(&info.invresttm.w[i], at, su32); at += su32;
                info.invresttm.x[i] = swap_endian<f32>(info.invresttm.x[i]);
                info.invresttm.y[i] = swap_endian<f32>(info.invresttm.y[i]);
                info.invresttm.z[i] = swap_endian<f32>(info.invresttm.z[i]);
                info.invresttm.w[i] = swap_endian<f32>(info.invresttm.w[i]);
            }

            return true;
        }

        bool read_buffer(const u8*& at, mesh*& info, u32& count) {
            u32 j{ 0 };
            node* Node = new node();
            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, *Node);
                // Assign Node Values
                info[i].name = Node->name;
                info[i].modeltm = Node->modeltm;
                info[i].nodeFlags = Node->nodeFlags;
                info[i].id = Node->id;
                info[i].parentIndex = Node->parentIndex;

                memcpy(&(info[i].primCount), at, su32); at += su32;
                info[i].primCount = swap_endian<u32>(info[i].primCount);
                
                if (info[i].primCount > 0) {
                    info[i].primIndex = new u32[info[i].primCount];
                    memcpy(info[i].primIndex, at, su32 * info[i].primCount); at += su32 * info[i].primCount;
                    for (j = 0;j < info[i].primCount;++j) {
                        info[i].primIndex[j] = swap_endian<u32>(info[i].primIndex[j]);
                    }
                }
                
                memcpy(&(info[i].meshboneCount), at, su32); at += su32;
                info[i].meshboneCount = swap_endian<u32>(info[i].meshboneCount);

                if (info[i].meshboneCount > 0) {
                    info[i].meshbone = new meshbone[info[i].meshboneCount];
                    for (j = 0;j < info[i].meshboneCount;++j) {
                        read_buffer(at, info[i].meshbone[j]);
                    }
                }
            }
            return true;
        }

        bool read_buffer(const u8*& at, camera*& info, u32& count) {
            node* Node = new node();
            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, *Node);
                // Assign Node Values
                info[i].name = Node->name;
                info[i].modeltm = Node->modeltm;
                info[i].nodeFlags = Node->nodeFlags;
                info[i].id = Node->id;
                info[i].parentIndex = Node->parentIndex;

                memcpy(&(info[i].front), at, su32); at += su32; info[i].front = swap_endian<f32>(info[i].front);
                memcpy(&(info[i].back), at, su32); at += su32; info[i].back = swap_endian<f32>(info[i].back);
                memcpy(&(info[i].FOV), at, su32); at += su32; info[i].FOV = swap_endian<f32>(info[i].FOV);
            }
            return true;
        }

        bool read_buffer(const u8*& at, light*& info, u32& count) {
            node* Node = new node();
            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, *Node);
                // Assign Node Values
                info[i].name = Node->name;
                info[i].modeltm = Node->modeltm;
                info[i].nodeFlags = Node->nodeFlags;
                info[i].id = Node->id;
                info[i].parentIndex = Node->parentIndex;

                memcpy(&info[i].colour.x, at, su32 * 3); at += su32 * 3;
                for (u32 j = 0;j < 3;++j) {
                    info[i].colour.x[j] = swap_endian<f32>(info[i].colour.x[j]);
                }

                memcpy(&(info[i].reserved1), at, su32); at += su32; info[i].reserved1 = swap_endian<f32>(info[i].reserved1);
                memcpy(&(info[i].reserved2), at, su32); at += su32; info[i].reserved1 = swap_endian<f32>(info[i].reserved1);
                memcpy(&(info[i].farAttenStart), at, su32); at += su32; info[i].farAttenStart = swap_endian<f32>(info[i].farAttenStart);
                memcpy(&(info[i].farAttenEnd), at, su32); at += su32; info[i].farAttenEnd = swap_endian<f32>(info[i].farAttenEnd);
                memcpy(&(info[i].inner), at, su32); at += su32; info[i].inner = swap_endian<f32>(info[i].inner);
                memcpy(&(info[i].outer), at, su32); at += su32; info[i].outer = swap_endian<f32>(info[i].outer);
                memcpy(&(info[i].type), at, 1); at += 1; info[i].type = swap_endian<u8>(info[i].type);
            }
            return true;
        }

        bool read_buffer(const u8*& at, dummy*& info, u32& count) {
            u32 j{ 0 };
            node* Node = new node();
            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, *Node);
                // Assign Node Values
                info[i].name = Node->name;
                info[i].modeltm = Node->modeltm;
                info[i].nodeFlags = Node->nodeFlags;
                info[i].id = Node->id;
                info[i].parentIndex = Node->parentIndex;

                memcpy(&(info[i].boxMin.x), at, su32 * 3); at += su32 * 3;
                for (j = 0;j < 3;++j) {
                    info[i].boxMin.x[j] = swap_endian<f32>(info[i].boxMin.x[j]);
                }
                memcpy(&(info[i].boxMax.x), at, su32 * 3); at += su32 * 3;
                for (j = 0;j < 3;++j) {
                    info[i].boxMax.x[j] = swap_endian<f32>(info[i].boxMax.x[j]);
                }
            }
            return true;
        }

        bool read_buffer(const u8*& at, line& info) {
            u32 j{ 0 };

            memcpy(&(info.start.x), at, su32 * 3); at += su32 * 3;
            for (j = 0;j < 3;++j) {
                info.start.x[j] = swap_endian<f32>(info.start.x[j]);
            }
            memcpy(&(info.end.x), at, su32 * 3); at += su32 * 3;
            for (j = 0;j < 3;++j) {
                info.end.x[j] = swap_endian<f32>(info.end.x[j]);
            }

            return true;
        }

        bool read_buffer(const u8*& at, path& info) {
            memcpy(&(info.beginLine), at, su32); at += su32; info.beginLine = swap_endian<s32>(info.beginLine);
            memcpy(&(info.endLine), at, su32); at += su32; info.endLine = swap_endian<s32>(info.endLine);

            return true;
        }

        bool read_buffer(const u8*& at, shape*& info, u32& count) {
            node* Node = new node();
            u32 j{ 0 };
            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, *Node);
                // Assign Node Values
                info[i].name = Node->name;
                info[i].modeltm = Node->modeltm;
                info[i].nodeFlags = Node->nodeFlags;
                info[i].id = Node->id;
                info[i].parentIndex = Node->parentIndex;

                memcpy(&(info[i].lineCount), at, su32); at += su32; info[i].lineCount = swap_endian<s32>(info[i].lineCount);
                memcpy(&(info[i].pathCount), at, su32); at += su32; info[i].pathCount = swap_endian<s32>(info[i].pathCount);

                info[i].lines = new line[info[i].lineCount];
                info[i].paths = new path[info[i].pathCount];

                for (j = 0;j < info[i].lineCount;j++) {
                    read_buffer(at, info[i].lines[j]);
                }
                for (j = 0;j < info[i].pathCount;j++) {
                    read_buffer(at, info[i].paths[j]);
                }
            }
            return true;
        }

        bool read_buffer(const u8*& at, keyframeSequence& info) {
            u32 size{ 0 };
            memcpy(&(info.keyCount), at, su32); at += su32; info.keyCount = swap_endian<s32>(info.keyCount);

            memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
            info.dataFormat.assign(at, at + size); at += size; // format without "DF_" prefix
            info.dataFormat = "DF_" + info.dataFormat;

            u32 length{ 0 };

            switch (dfMap[info.dataFormat]) { // i think i should build a different class for this.
            case DF_S_16:
                size = info.keyCount;
                length = su16;
                break;
            
            case DF_V2_16:
                size = info.keyCount * 2;
                length = su16;
                break;
            
            case DF_V3_16:
                size = info.keyCount * 3;
                length = su16;
                break;
            
            case DF_V4_16:
                size = info.keyCount * 4;
                length = su16;
                break;
            
            default:
                assert(false && "Unimplemented type: Go into Debugger Mode to check the format!");
                return false;
            }

            memcpy(&(info.scale), at, su32); at += su32;
            info.scale = swap_endian<f32>(info.scale);
            memcpy(&(info.bias), at, su32 * 3); at += su32 * 3;

            for (int j{ 0 };j < 3;++j) {
                info.bias[j] = swap_endian<f32>(info.bias[j]);
            }

            info.keys = new f32[size];

            memcpy(info.keys, at, size * length); at += size * length;
            for (u32 j{ 0 };j < size;++j) {
                info.keys[j] = swap_endian<s16>(info.keys[j]); // is it supposed to be little or big endian?
            }
            info.size = size;

            return true;
        }

        bool read_buffer(const u8*& at, transformAnimation*& info, u32& count) {
            u16 size{ 0 }; u32 j{ 0 };
            for (u32 i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].nodeName.assign(at, at + size); at += size; // name

                memcpy(&(info[i].posKeyRate), at, 1); at += 1; info[i].posKeyRate = swap_endian<u8>(info[i].posKeyRate);
                memcpy(&(info[i].rotKeyRate), at, 1); at += 1; info[i].rotKeyRate = swap_endian<u8>(info[i].rotKeyRate);
                memcpy(&(info[i].sclKeyRate), at, 1); at += 1; info[i].sclKeyRate = swap_endian<u8>(info[i].sclKeyRate);
                memcpy(&(info[i].endBehaviour), at, 1); at += 1; info[i].endBehaviour = swap_endian<u8>(info[i].endBehaviour);

                info[i].posKeyData = new keyframeSequence();
                info[i].rotKeyData = new keyframeSequence();
                info[i].sclKeyData = new keyframeSequence();

                read_buffer(at, *info[i].posKeyData);
                read_buffer(at, *info[i].rotKeyData);
                read_buffer(at, *info[i].sclKeyData);
            }
            return true;
        }

        bool read_buffer(const u8*& at, userProperty*& info, u32& count) {
            u16 size{ 0 }; u32 j{ 0 };
            for (u32 i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].nodeName.assign(at, at + size); at += size; // name

                memcpy(&size, at, su16); at += su16; size = swap_endian<u16>(size);
                info[i].propertyText.assign(at, at + size); at += size; // property text
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
        entityInfo.Primitive_Count = swap_endian<u32>(entityInfo.Primitive_Count);
        primitive_info* Primitives = new primitive_info[entityInfo.Primitive_Count];
        read_buffer(at, Primitives, entityInfo.Primitive_Count);

        check_id(at, *header);

        memcpy(&(entityInfo.Mesh_Count), at, su32); at += su32;
        entityInfo.Mesh_Count = swap_endian<u32>(entityInfo.Mesh_Count);
        mesh* Meshes = new mesh[entityInfo.Mesh_Count];
        read_buffer(at, Meshes, entityInfo.Mesh_Count);

        check_id(at, *header);

        memcpy(&(entityInfo.Camera_Count), at, su32); at += su32;
        entityInfo.Camera_Count = swap_endian<u32>(entityInfo.Camera_Count);
        camera* Cameras;
        if (entityInfo.Camera_Count > 0) {
            Cameras = new camera[entityInfo.Camera_Count];
            read_buffer(at, Cameras, entityInfo.Camera_Count);
        }
        
        check_id(at, *header);

        memcpy(&(entityInfo.Light_Count), at, su32); at += su32;
        entityInfo.Light_Count = swap_endian<u32>(entityInfo.Light_Count);
        light* Lights;
        if (entityInfo.Light_Count > 0) {
            Lights = new light[entityInfo.Light_Count];
            read_buffer(at, Lights, entityInfo.Light_Count);
        }

        check_id(at, *header);

        memcpy(&(entityInfo.Dummy_Count), at, su32); at += su32;
        entityInfo.Dummy_Count = swap_endian<u32>(entityInfo.Dummy_Count);
        dummy* Dummies;
        if (entityInfo.Dummy_Count > 0) {
            Dummies = new dummy[entityInfo.Dummy_Count];
            read_buffer(at, Dummies, entityInfo.Dummy_Count);
        }

        check_id(at, *header);

        memcpy(&(entityInfo.Shape_Count), at, su32); at += su32;
        entityInfo.Shape_Count = swap_endian<u32>(entityInfo.Shape_Count);
        shape* Shapes;
        if (entityInfo.Shape_Count > 0) {
            Shapes = new shape[entityInfo.Shape_Count];
            read_buffer(at, Shapes, entityInfo.Shape_Count);
        }

        check_id(at, *header);

        memcpy(&(entityInfo.OtherNodes_Count), at, su32); at += su32;
        entityInfo.OtherNodes_Count = swap_endian<u32>(entityInfo.OtherNodes_Count);
        node* Nodes;
        if (entityInfo.OtherNodes_Count > 0) {
            Nodes = new node[entityInfo.OtherNodes_Count];
            for (u32 i{ 0 };i < entityInfo.OtherNodes_Count; ++i) {
                read_buffer(at, Nodes[i]);
            }
        }

        check_id(at, *header);

        memcpy(&(entityInfo.TransformAnimation_Count), at, su32); at += su32;
        entityInfo.TransformAnimation_Count = swap_endian<u32>(entityInfo.TransformAnimation_Count);
        transformAnimation* TransformAnimations;
        if (entityInfo.TransformAnimation_Count > 0) {
            TransformAnimations = new transformAnimation[entityInfo.TransformAnimation_Count];
            for (u32 i{ 0 };i < entityInfo.TransformAnimation_Count; ++i) {
                read_buffer(at, TransformAnimations, entityInfo.TransformAnimation_Count);
            }
        }

        check_id(at, *header);

        memcpy(&(entityInfo.UserProperties_Count), at, su32); at += su32;
        entityInfo.UserProperties_Count = swap_endian<u32>(entityInfo.UserProperties_Count);
        userProperty* UserProperties;
        if (entityInfo.UserProperties_Count > 0) {
            UserProperties = new userProperty[entityInfo.UserProperties_Count];
            read_buffer(at, UserProperties, entityInfo.UserProperties_Count);
        }

        //std::unique_ptr<hgr::mesh> Mesh{};
        mesh* Mesh = new mesh();
        Mesh->name = "James";

        //delete Mesh; // to avoid memory leaks
        return true;
    }
}