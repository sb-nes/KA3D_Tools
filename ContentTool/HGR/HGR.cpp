#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>

#include "HGR.h"
#include "../ToolCommon.h"
#include "Entity.h"
#include "../FBXExporter.h"

#define MAX_TEXCOORDS = 4 

namespace tools::hgr {
	namespace {
        constexpr u32 su16{ sizeof(u16) }; // 2 bytes for reading
        constexpr u32 su32{ sizeof(u32) }; // 4 bytes for reading

        u16 version{ 0 };

        constexpr bool is_big_endian = (std::endian::native == std::endian::big);

        bool check_signature(const u8*& at) {
            // Check Signature
            char magic[4]; memcpy(magic, at, 4);
            //int z = memcmp(magic, "hgrfi", 4);
            //if (str != "hgrf") return false; // Fails to check RN
            at += 5;

            return true;
        }

        bool check_id(const u8*& at, hgr_info& info) {
            u32 id{ ++info.check_id };
            memcpy(&(info.check_id), at, su32); at += su32; 
            //info.check_id = swap_endian<u32>(info.check_id);
            SWAP(info.check_id, u32);

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
                memcpy(&size, at, su16); at += su16; 
                SWAP(size,u16);
                info[i].name.assign(at, at + size); at += size;
                memcpy(&(info[i].type), at, su32); at += su32;
            }
            return true;
        }

        bool read_buffer(const u8*& at, texParam*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; 
                SWAP(size, u16);
                info[i].param_type.assign(at, at + size); at += size; // param Type

                memcpy(&(info[i].texIndex), at, su16); at += su16; 
                SWAP(info[i].texIndex, u16);
            }
            return true;
        }

        bool read_buffer(const u8*& at, vec4Param*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; 
                SWAP(size, u16);
                info[i].param_type.assign(at, at + size); at += size; // param Type

                memcpy(&(info[i].value), at, su32 * 4); at += su32 * 4; 
                SWAP(info[i].value[0], f32);
                SWAP(info[i].value[1], f32);
                SWAP(info[i].value[2], f32);
                SWAP(info[i].value[3], f32);
            }
            return true;
        }

        bool read_buffer(const u8*& at, floatParam*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; 
                SWAP(size, u16);
                info[i].param_type.assign(at, at + size); at += size; // param Type

                memcpy(&(info[i].value), at, su32); at += su32; 
                SWAP(info[i].value, f32);
            }
            return true;
        }

        bool read_buffer(const u8*& at, material_info*& info, u32& count) {
            u16 size{ 0 };
            for (u32 i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; 
                SWAP(size, u16);
                info[i].name.assign(at, at + size); at += size; // name

                memcpy(&size, at, su16); at += su16; 
                SWAP(size, u16);
                info[i].shaderName.assign(at, at + size); at += size; // shaderName

                memcpy(&(info[i].lightmap_info), at, su32); at += su32; 
                SWAP(info[i].lightmap_info, s32);

                memcpy(&(info[i].texParamCount), at, 1); at += 1;
                SWAP(info[i].texParamCount, u8);
                info[i].TexParams = new texParam[info[i].texParamCount];
                read_buffer(at, info[i].TexParams, info[i].texParamCount);

                memcpy(&(info[i].vec4ParamCount), at, 1); at += 1; 
                SWAP(info[i].vec4ParamCount, u8);
                info[i].Vec4Params = new vec4Param[info[i].vec4ParamCount];
                read_buffer(at, info[i].Vec4Params, info[i].vec4ParamCount);

                memcpy(&(info[i].floatParamCount), at, 1); at += 1; 
                SWAP(info[i].floatParamCount, u8);
                info[i].FloatParams = new floatParam[info[i].floatParamCount];
                read_buffer(at, info[i].FloatParams, info[i].floatParamCount);
            }
            return true;
        }

        bool read_buffer(const u8*& at, vertFormat*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
                info[i].type.assign(at, at + size); at += size; // type with "DT_" prefix

                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
                info[i].format.assign(at, at + size); at += size; // format without "DF_" prefix
                //info[i].format = "DF_" + info[i].format;
            }
            return true;
        }

        bool read_buffer(const u8*& at, vertArray*& info, u8& count, u32 verts, vertFormat* formats) {
            u32 size{ 0 };
            u32 length{ 0 };

            f32 posscalebias[4]{1,0,0,0};
            f32 uvscalebias[4]{1,0,0,0};
            if (version >= 190) {
                // posscalebias = readFloat4();
                memcpy(&(posscalebias), at, su32 * 4); at += su32 * 4;
                for (int i{ 0 };i < 4;++i) SWAP(posscalebias[i], f32);

                // uvscalebias = readFloat4();
                memcpy(&(uvscalebias), at, su32 * 4); at += su32 * 4;
                for (int i{ 0 };i < 4;++i) SWAP(uvscalebias[i], f32);
            }

            for (int i{ 0 };i < count;++i) {
                //memcpy(&(info[i].scale), at, su32); at += su32;
                //SWAP(info[i].scale, f32);
                //memcpy(&(info[i].bias), at, su32 * 3); at += su32 * 3;

                if (version < 190) {
                    // dummy scale + bias4 -> discarded data
                    at += su32;
                    at += su32 * 4;
                }

                size = VertexFormat::getDataDim(VertexFormat::toDataFormat( formats[i].format.c_str() ));
                length = VertexFormat::getDataSize(VertexFormat::toDataFormat(formats[i].format.c_str()));
                length /= size;
                size *= verts;

                info[i].value = new s16[size];

                memcpy(info[i].value, at, size * length); at += size * length; // Lets see with little endian
                //for (u32 j{ 0 };j < size;++j) {
                //    SWAP(info[i].value[j], s16);
                //}

                // Copy pos and uv to respective datatype channels
                if (VertexFormat::toDataType(formats[i].type.c_str()) == VertexFormat::DT_POSITION) {
                    info[i].scale = posscalebias[0];
                    info[i].bias[0] = posscalebias[1];
                    info[i].bias[1] = posscalebias[2];
                    info[i].bias[2] = posscalebias[3];
                } else if (VertexFormat::toDataType(formats[i].type.c_str()) == VertexFormat::DT_TEX0) {
                    info[i].scale = uvscalebias[0];
                    info[i].bias[0] = uvscalebias[1];
                    info[i].bias[1] = uvscalebias[2];
                    info[i].bias[2] = uvscalebias[3];
                }

                // Set Bounds of Primitive - I'm not reading, so i can fix this later in a different area
                if (VertexFormat::toDataType(formats[i].type.c_str()) == VertexFormat::DT_POSITION) {
                    math::float4 boundmin, boundmax;
                    float boundradius;
                    //VertexFormat::getBound(&buf[0], df, verts, posscalebias, &boundmin, &boundmax, &boundradius);
                    //prim->setBound(boundmin.xyz(), boundmax.xyz(), boundradius);
                }

                info[i].size = size;
            }
            return true;
        }

        bool read_buffer(const u8*& at, primitive_info*& info, u32& count) {
            for (u32 i{ 0 };i < count;++i) {
                if (version < 190) {
                    memcpy(&(info[i].verts), at, su16); at += su16;
                    SWAP(info[i].verts, u32);
                    memcpy(&(info[i].indices), at, su16); at += su16;
                    SWAP(info[i].indices, u32);
                } else {
                    memcpy(&(info[i].verts), at, su32); at += su32;
                    SWAP(info[i].verts, u32);
                    memcpy(&(info[i].indices), at, su32); at += su32;
                    SWAP(info[i].indices, u32);
                }

                memcpy(&(info[i].formatCount), at, 1); at += 1;
                info[i].formats = new vertFormat[info[i].formatCount]; 
                info[i].vArray = new vertArray[info[i].formatCount];

                read_buffer(at, info[i].formats, info[i].formatCount);

                memcpy(&(info[i].matIndex), at, su16); at += su16;
                SWAP(info[i].matIndex, u16);
                memcpy(&(info[i].primitiveType), at, su16); at += su16; // Primitive::PRIM_TRI -> Default
                SWAP(info[i].primitiveType, u16);

                read_buffer(at, info[i].vArray, info[i].formatCount, info[i].verts, info[i].formats);

                // WARNING: Endianess dependent read
                info[i].indexData = new u16[info[i].indices];
                memcpy(info[i].indexData, at, su16 * info[i].indices); at += su16 * info[i].indices;
                //for (u32 j{ 0 };j < info[i].indices;++j) {
                //    SWAP(info[i].indexData[j], u16);
                //}
                
                memcpy(&(info[i].usedBoneCount), at, 1); at += 1; 
                assert(info[i].usedBoneCount <= MAX_BONES && ("Failed to load scene. Too many bones: "+i));

                info[i].usedBones = new u8[info[i].usedBoneCount];
                memcpy(info[i].usedBones, at, info[i].usedBoneCount); at += info[i].usedBoneCount;
            }
            return true;
        }

        bool read_buffer(const u8*& at, node& info) {
            u16 size{ 0 }; u32 i{ 0 };
            memcpy(&size, at, su16); at += su16; 
            SWAP(size, u16);
            info.name.assign(at, at + size); at += size; // name

            // model tm
            for (i = 0;i < 3;++i) {
                memcpy(&info.modeltm.x[i], at, su32); at += su32; 
                memcpy(&info.modeltm.y[i], at, su32); at += su32;
                memcpy(&info.modeltm.z[i], at, su32); at += su32;
                memcpy(&info.modeltm.w[i], at, su32); at += su32;
                SWAP(info.modeltm.x[i], f32);
                SWAP(info.modeltm.y[i], f32);
                SWAP(info.modeltm.z[i], f32);
                SWAP(info.modeltm.w[i], f32);
            }

            memcpy(&(info.nodeFlags), at, su32); at += su32;
            SWAP(info.nodeFlags, u32);

            memcpy(&(info.id), at, su32); at += su32;
            SWAP(info.id, u32);

            memcpy(&(info.parentIndex), at, su32); at += su32; // u32_invalid_id = -1 -> iron-blooded orphan
            SWAP(info.parentIndex, u32);

            return true;
        }

        bool read_buffer(const u8*& at, meshbone& info) {
            u32 i{ 0 };

            memcpy(&(info.boneNodeIndex), at, su32); at += su32;
            SWAP(info.boneNodeIndex, u32);

            // invresttm
            for (i = 0;i < 3;++i) {
                memcpy(&info.invresttm.x[i], at, su32); at += su32;
                memcpy(&info.invresttm.y[i], at, su32); at += su32;
                memcpy(&info.invresttm.z[i], at, su32); at += su32;
                memcpy(&info.invresttm.w[i], at, su32); at += su32;
                SWAP(info.invresttm.x[i], f32);
                SWAP(info.invresttm.y[i], f32);
                SWAP(info.invresttm.z[i], f32);
                SWAP(info.invresttm.w[i], f32);
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
                SWAP(info[i].primCount, u32);
                
                if (info[i].primCount > 0) {
                    info[i].primIndex = new u32[info[i].primCount];
                    memcpy(info[i].primIndex, at, su32 * info[i].primCount); at += su32 * info[i].primCount;
                    for (j = 0;j < info[i].primCount;++j) {
                        SWAP(info[i].primIndex[j], u32);
                    }
                }
                
                memcpy(&(info[i].meshboneCount), at, su32); at += su32;
                SWAP(info[i].meshboneCount, u32);

                if (info[i].meshboneCount > 0) {
                    info[i].meshbone = new meshbone[info[i].meshboneCount];
                    for (j = 0;j < info[i].meshboneCount;++j) {
                        read_buffer(at, info[i].meshbone[j]);
                    }
                }
            }

            delete Node;
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

                memcpy(&(info[i].front), at, su32); at += su32;
                SWAP(info[i].front, f32);
                memcpy(&(info[i].back), at, su32); at += su32;
                SWAP(info[i].back, f32);
                memcpy(&(info[i].FOV), at, su32); at += su32;
                SWAP(info[i].FOV, f32);
            }
            delete Node;
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
                    SWAP(info[i].colour.x[j], f32);
                }

                memcpy(&(info[i].reserved1), at, su32); at += su32; 
                SWAP(info[i].reserved1, f32);
                memcpy(&(info[i].reserved2), at, su32); at += su32;
                SWAP(info[i].reserved2, f32);
                memcpy(&(info[i].farAttenStart), at, su32); at += su32;
                SWAP(info[i].farAttenStart, f32);
                memcpy(&(info[i].farAttenEnd), at, su32); at += su32;
                SWAP(info[i].farAttenEnd, f32);
                memcpy(&(info[i].inner), at, su32); at += su32;
                SWAP(info[i].inner, f32);
                memcpy(&(info[i].outer), at, su32); at += su32;
                SWAP(info[i].outer, f32);
                memcpy(&(info[i].type), at, 1); at += 1;
                SWAP(info[i].type, u8);
            }
            delete Node;
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
                    SWAP(info[i].boxMin.x[j], f32);
                }
                memcpy(&(info[i].boxMax.x), at, su32 * 3); at += su32 * 3;
                for (j = 0;j < 3;++j) {
                    SWAP(info[i].boxMax.x[j], f32);
                }
            }
            delete Node;
            return true;
        }

        bool read_buffer(const u8*& at, line& info) {
            u32 j{ 0 };

            memcpy(&(info.start.x), at, su32 * 3); at += su32 * 3;
            for (j = 0;j < 3;++j) {
                SWAP(info.start.x[j], f32);
            }
            memcpy(&(info.end.x), at, su32 * 3); at += su32 * 3;
            for (j = 0;j < 3;++j) {
                SWAP(info.end.x[j], f32);
            }

            return true;
        }

        bool read_buffer(const u8*& at, path& info) {
            memcpy(&(info.beginLine), at, su32); at += su32;
            SWAP(info.beginLine, s32);
            memcpy(&(info.endLine), at, su32); at += su32;
            SWAP(info.endLine, s32);

            return true;
        }

        bool read_buffer(const u8*& at, shape*& info, u32& count) {
            node* Node = new node();
            s32 j{ 0 };
            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, *Node);
                // Assign Node Values
                info[i].name = Node->name;
                info[i].modeltm = Node->modeltm;
                info[i].nodeFlags = Node->nodeFlags;
                info[i].id = Node->id;
                info[i].parentIndex = Node->parentIndex;

                memcpy(&(info[i].lineCount), at, su32); at += su32;
                SWAP(info[i].lineCount, s32);
                memcpy(&(info[i].pathCount), at, su32); at += su32;
                SWAP(info[i].pathCount, s32);

                info[i].lines = new line[info[i].lineCount];
                info[i].paths = new path[info[i].pathCount];

                for (j = 0;j < info[i].lineCount;j++) {
                    read_buffer(at, info[i].lines[j]);
                }
                for (j = 0;j < info[i].pathCount;j++) {
                    read_buffer(at, info[i].paths[j]);
                }
            }
            delete Node;
            return true;
        }

        bool read_buffer(const u8*& at, keyframeSequence& info) {
            u16 s{ 0 };
            u32 size{ 0 };
            u32 length{ 0 };

            memcpy(&(info.keyCount), at, su32); at += su32; info.keyCount = swap_endian<s32>(info.keyCount);

            if (version < 192) {
                memcpy(&s, at, su16); at += su16; s = swap_endian<u16>(s);
                info.dataFormat.assign(at, at + s); at += s; // format without "DF_" prefix

                memcpy(&(info.scale), at, su32); at += su32;
                info.scale = swap_endian<f32>(info.scale);
                memcpy(&(info.bias), at, su32 * 3); at += su32 * 3;

                for (int j{ 0 };j < 3;++j) {
                    info.bias[j] = swap_endian<f32>(info.bias[j]);
                }

                size = VertexFormat::getDataDim(VertexFormat::toDataFormat(info.dataFormat.c_str()));
                length = VertexFormat::getDataSize(VertexFormat::toDataFormat(info.dataFormat.c_str()));
                length /= size;
                size *= info.keyCount;

                info.keys = new f32[size];

                memcpy(info.keys, at, size * length); at += size * length;
                for (u32 j{ 0 };j < size;++j) {
                    info.keys[j] = swap_endian<f32>(info.keys[j]); // is it supposed to be little or big endian?
                }
                info.size = size;

            } else { // Implement in v193
                int dim;
                memcpy(&dim, at, su32); at += su32; dim= swap_endian<u32>(dim);
                assert((dim == 4 || dim == 3) && "Keyframe sequence in {0} dimension invalid ({1})");

                if (dim == 4) { // VertexFormat::DF_V4_32
                    info.dataFormat = "DF_V4_32"; // float32[4]
                    //obj = new KeyframeSequence(keys, VertexFormat::DF_V4_32);
                    //readFloat4Array16((float4*)obj->data(), keys);
                } else {
                    info.dataFormat = "DF_V3_32"; // float32[3]
                    //obj = new KeyframeSequence(keys, VertexFormat::DF_V3_32);
                    //readFloat3Array16((float3*)obj->data(), keys);
                }
            }

            return true;
        }

        bool read_float3anim(const u8*& at, float3Animation& info) {
            u16 s{ 0 };
            memcpy(&(info.keyCount), at, su32); at += su32; info.keyCount = swap_endian<s32>(info.keyCount);

            return true;
        }

        bool read_buffer(const u8*& at, transformAnimation*& info, u32& count) {
            u16 size{ 0 };
            for (u32 i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16; 
                SWAP(size, u16);
                info[i].nodeName.assign(at, at + size); at += size; // name

                memcpy(&(info[i].posKeyRate), at, 1); at += 1;
                memcpy(&(info[i].rotKeyRate), at, 1); at += 1;
                memcpy(&(info[i].sclKeyRate), at, 1); at += 1;
                memcpy(&(info[i].endBehaviour), at, 1); at += 1;

                assert(info[i].endBehaviour < BehaviourType::BEHAVIOUR_COUNT);

                // not listed in the hgr file format documentation
                if (version >= 192) memcpy(&info->isOptimized, at, 1); at += 1;

                if (!info->isOptimized) {
                    // not implementing rn

                    //info[i].posKeyData = new keyframeSequence();
                    info[i].rotKeyData = new keyframeSequence();
                    //info[i].sclKeyData = new keyframeSequence();

                    //read_buffer(at, *info[i].posKeyData);
                    read_buffer(at, *info[i].rotKeyData);
                    //read_buffer(at, *info[i].sclKeyData);
                } else { // New Implementation

                    info[i].rotKeyData = new keyframeSequence(); // Only this remains same

                    if (version >= 193) {
                        memcpy(&(info[i].endTime), at, su32); at += su32;
                        info[i].endTime = swap_endian<f32>(info[i].endTime);
                    } else {
                        info[i].endTime = float(info[i].rotKeyData->keys[0]) * float(info[i].rotKeyRate);
                    }
                }
            }
            return true;
        }

        bool read_buffer(const u8*& at, userProperty*& info, u32& count) {
            u16 size{ 0 };
            for (u32 i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
                info[i].nodeName.assign(at, at + size); at += size; // name

                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
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

        version = header->m_ver;

        scene_param_info* sceneParams = new scene_param_info();
        read_buffer(at, *sceneParams);

        memcpy(&(header->check_id), at, su32); at += su32;
        SWAP(header->check_id, u32);

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
        camera* Cameras = new camera[entityInfo.Camera_Count];
        if (entityInfo.Camera_Count > 0) {
            read_buffer(at, Cameras, entityInfo.Camera_Count);
        }
        
        check_id(at, *header);
        
        memcpy(&(entityInfo.Light_Count), at, su32); at += su32;
        entityInfo.Light_Count = swap_endian<u32>(entityInfo.Light_Count);
        light* Lights = new light[entityInfo.Light_Count];
        if (entityInfo.Light_Count > 0) {
            read_buffer(at, Lights, entityInfo.Light_Count);
        }
        
        check_id(at, *header);
        
        memcpy(&(entityInfo.Dummy_Count), at, su32); at += su32;
        entityInfo.Dummy_Count = swap_endian<u32>(entityInfo.Dummy_Count);
        dummy* Dummies = new dummy[entityInfo.Dummy_Count];
        if (entityInfo.Dummy_Count > 0) {
            read_buffer(at, Dummies, entityInfo.Dummy_Count);
        }
        
        check_id(at, *header);
        
        memcpy(&(entityInfo.Shape_Count), at, su32); at += su32;
        entityInfo.Shape_Count = swap_endian<u32>(entityInfo.Shape_Count);
        shape* Shapes = new shape[entityInfo.Shape_Count];
        if (entityInfo.Shape_Count > 0) {
            read_buffer(at, Shapes, entityInfo.Shape_Count);
        }
        
        check_id(at, *header);
        
        memcpy(&(entityInfo.OtherNodes_Count), at, su32); at += su32;
        entityInfo.OtherNodes_Count = swap_endian<u32>(entityInfo.OtherNodes_Count);
        node* Nodes = new node[entityInfo.OtherNodes_Count];
        if (entityInfo.OtherNodes_Count > 0) {
            for (u32 i{ 0 };i < entityInfo.OtherNodes_Count; ++i) {
                read_buffer(at, Nodes[i]);
            }
        }
        
        check_id(at, *header);

        // NOTE: Implement Animations Later
        /*
        memcpy(&(entityInfo.TransformAnimation_Count), at, su32); at += su32;
        entityInfo.TransformAnimation_Count = swap_endian<u32>(entityInfo.TransformAnimation_Count);
        transformAnimation* TransformAnimations = new transformAnimation[entityInfo.TransformAnimation_Count];
        if (entityInfo.TransformAnimation_Count > 0) {
            for (u32 i{ 0 };i < entityInfo.TransformAnimation_Count; ++i) {
                read_buffer(at, TransformAnimations, entityInfo.TransformAnimation_Count);
            }
        }

        check_id(at, *header);

        memcpy(&(entityInfo.UserProperties_Count), at, su32); at += su32;
        entityInfo.UserProperties_Count = swap_endian<u32>(entityInfo.UserProperties_Count);
        userProperty* UserProperties  = new userProperty[entityInfo.UserProperties_Count];
        if (entityInfo.UserProperties_Count > 0) {
            read_buffer(at, UserProperties, entityInfo.UserProperties_Count);
        }
        */

        assetData Asset{};
        // Fill Data
        Asset.info = header;
        Asset.scene_param = sceneParams;
        Asset.entityInfo = &entityInfo;
        Asset.texInfo = Textures;
        Asset.matInfo = Materials;
        Asset.primInfo = Primitives;

        CreateFBX(Asset, path); // FBX Exporter

        // to avoid memory leaks
        u32 i{ 0 };
        {
            delete header;
            delete sceneParams;
            delete[] Textures;
            for (i = 0;i < entityInfo.Material_Count;++i) {
                delete[] Materials[i].TexParams;
                delete[] Materials[i].Vec4Params;
                delete[] Materials[i].FloatParams;
            }
            delete[] Materials;
            for (i = 0;i < entityInfo.Primitive_Count;++i) {
                delete[] Primitives[i].formats;
                for (u32 j{ 0 };j < Primitives[i].formatCount;++j) {
                    delete[] Primitives[i].vArray[j].value;
                }
                delete[] Primitives[i].vArray;
                delete[] Primitives[i].indexData;
                delete[] Primitives[i].usedBones;
            }
            delete[] Primitives;
            for (i = 0;i < entityInfo.Mesh_Count;++i) {
                delete[] Meshes[i].primIndex;
                delete[] Meshes[i].meshbone;
            }
            delete[] Meshes;
            delete[] Cameras;
            delete[] Lights;
            delete[] Dummies;
            for (i = 0;i < entityInfo.Shape_Count;++i) {
                delete[] Shapes[i].lines;
                delete[] Shapes[i].paths;
            }
            delete[] Shapes;
            delete[] Nodes;
            //delete[] TransformAnimations;
            //delete[] UserProperties;
        }

        return true;
    }
}