#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>

#include "HGR.h"
#include "../ToolCommon.h"
#include "Entity.h"
#include "../FBXExporter.h"

namespace tools::hgr {

    namespace {
        constexpr u32 su16{ sizeof(u16) }; // 2 bytes for reading
        constexpr u32 su32{ sizeof(u32) }; // 4 bytes for reading

        u16 version{ 0 };
        bool corrupt{ false };
        std::vector<node> entityNodes;
        entity_info entityInfo{};

        constexpr bool is_big_endian = (std::endian::native == std::endian::big);

        bool check_signature(const u8*& at) {
            // Check Signature
            char magic[5]; memcpy(magic, at, 5);
            std::string str = "hgrf";
            //int z = memcmp(magic, "hgrfi", 4);
            //if (magic != str.c_str()) return false; // Fails to check RN
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

        bool read_buffer(const u8*& at, std::vector<texture_info>& info, u32& count) {
            u16 size{ 0 };
            texture_info t{};
            for (u32 i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
                t.name.assign(at, at + size); at += size;
                memcpy(&(t.type), at, su32); at += su32;

                info.emplace_back(t);
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
                if (info[i].texIndex > entityInfo.Texture_Count) {
                    assert(info[i].texIndex > entityInfo.Texture_Count);
                    return false;
                }
            }
            return true;
        }

        bool read_buffer(const u8*& at, vec4Param*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
                info[i].param_type.assign(at, at + size); at += size; // param Type

                if (corrupt) { // error case
                    if (info[i].param_type == "AM>9ENTC") {
                        info[i].param_type = "AMBIENTC";
                    }
                    if (info[i].param_type == "SPEC³¼?RC") {
                        info[i].param_type = "SPECULARC";
                    }
                    if (info[i].param_type == "S°½ÃULARC") {
                        info[i].param_type = "SPECULARC";
                    }

                }

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

        bool read_buffer(const u8*& at, std::vector<material_info>& info, u32& count) {
            u16 size{ 0 };
            material_info m{};
            for (u32 i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
                m.name.assign(at, at + size); at += size; // name

                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
                m.shaderName.assign(at, at + size);
                if (corrupt) {
                    if (size == 249 && m.shaderName.starts_with("Ó")) {
                        size = 9;
                        m.shaderName.assign(at, at + size);
                    }
                }
                at += size; // shaderName

                memcpy(&(m.lightmap_info), at, su32); at += su32;
                SWAP(m.lightmap_info, s32);

                memcpy(&(m.texParamCount), at, 1); at += 1;
                SWAP(m.texParamCount, u8);
                m.TexParams = new texParam[m.texParamCount];
                assert(read_buffer(at, m.TexParams, m.texParamCount));

                memcpy(&(m.vec4ParamCount), at, 1); at += 1;
                SWAP(m.vec4ParamCount, u8);
                m.Vec4Params = new vec4Param[m.vec4ParamCount];
                read_buffer(at, m.Vec4Params, m.vec4ParamCount);

                memcpy(&(m.floatParamCount), at, 1); at += 1;
                SWAP(m.floatParamCount, u8);
                m.FloatParams = new floatParam[m.floatParamCount];
                read_buffer(at, m.FloatParams, m.floatParamCount);

                info.emplace_back(m);
            }
            return true;
        }

        bool read_buffer(const u8*& at, vertFormat*& info, u8& count) {
            u16 size{ 0 };
            for (int i{ 0 };i < count;++i) {
                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
                info[i].type.assign(at, at + size); // type with "DT_" prefix

                if (corrupt) { // ERROR CASES
                    if (info[i].type == "DT_") {
                        size = 5;
                        info[i].type.assign(at, at + size);
                    }
                    if (info[i].type == "DT_TE") {
                        size = 7;
                        info[i].type.assign(at, at + size);
                    }
                    if (info[i].type == "¼4WÌEX0") {
                        info[i].type = "DT_TEX0";
                    }
                    if (info[i].type == "DT_PO") {
                        size = 11;
                        info[i].type.assign(at, at + size);
                    }
                    if (info[i].type == "Ä4+POSITION") {
                        info[i].type = "DT_POSITION";
                    }
                    if (info[i].type == "DT_POSITIOJ") {
                        at += size;
                        size = 5; at += su16;
                        info[i].format.assign(at, at + size); at += size;
                        continue;
                    }
                    if (info[i].type == "ÄÌ_PO") {
                        size = 11;
                        info[i].type = "DT_POSITION";
                    }
                    if (info[i].type == "DT_TEX\x10") {
                        info[i].type = "DT_TEX0";
                    }
                }

                at += size;

                memcpy(&size, at, su16); at += su16;
                SWAP(size, u16);
                info[i].format.assign(at, at + size); // format without "DF_" prefix
                //info[i].format = "DF_" + info[i].format;
                if (corrupt) { // ERROR CASES
                    if (info[i].format == "V3_Ñ\n") {
                        info[i].format = "V3_16";
                    }
                    if (info[i].format.substr(0, 3) == "VîË") {
                        info[i].format = "V2_16";
                    }
                    if (info[i].format == "VîË") {
                        info[i].format = "V2_16";
                    }
                }
                at += size;
            }
            return true;
        }

        bool read_buffer(const u8*& at, vertArray*& info, u8& count, u32 verts, vertFormat* formats) {
            u32 size{ 0 };
            u32 length{ 0 };

            f32 posscalebias[4]{ 1,0,0,0 };
            f32 uvscalebias[4]{ 1,0,0,0 };
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

                size = VertexFormat::getDataDim(VertexFormat::toDataFormat(formats[i].format.c_str()));
                if (formats[i].type == "DT_POSITIOJ") {
                    formats[i].type = "DT_POSITION";
                }
                assert(size > 0);
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
                }
                else if (VertexFormat::toDataType(formats[i].type.c_str()) == VertexFormat::DT_TEX0) {
                    info[i].scale = uvscalebias[0];
                    info[i].bias[0] = uvscalebias[1];
                    info[i].bias[1] = uvscalebias[2];
                    info[i].bias[2] = uvscalebias[3];
                }

                // Set Bounds of Primitive - I'm not reading rn, so i can fix this later in a different area
                if (VertexFormat::toDataType(formats[i].type.c_str()) == VertexFormat::DT_POSITION) {
                    //math::float4 boundmin, boundmax;
                    //float boundradius;
                    //VertexFormat::getBound(&buf[0], df, verts, posscalebias, &boundmin, &boundmax, &boundradius);
                    //prim->setBound(boundmin.xyz(), boundmax.xyz(), boundradius);
                }

                info[i].size = size;
            }
            return true;
        }

        // TODO : Fix UV Mapping
        bool read_buffer(const u8*& at, std::vector<primitive_info>& info, u32& count) {
            primitive_info p{};
            for (u32 i{ 0 };i < count;++i) {
                if (version < 190) {
                    memcpy(&(p.verts), at, su16); at += su16;
                    SWAP(p.verts, u32);
                    memcpy(&(p.indices), at, su16); at += su16;
                    SWAP(p.indices, u32);
                }
                else {
                    memcpy(&(p.verts), at, su32); at += su32;
                    SWAP(p.verts, u32);
                    memcpy(&(p.indices), at, su32); at += su32;
                    SWAP(p.indices, u32);
                }

                memcpy(&(p.formatCount), at, 1); at += 1;
                p.formats = new vertFormat[p.formatCount];
                p.vArray = new vertArray[p.formatCount];

                read_buffer(at, p.formats, p.formatCount);

                if (p.verts == 24 && p.indices == 162 && corrupt) { // Error Case
                    p.verts = 56;
                }

                memcpy(&(p.matIndex), at, su16); at += su16;
                SWAP(p.matIndex, u16);
                memcpy(&(p.primitiveType), at, su16); at += su16; // Primitive::PRIM_TRI -> Default
                SWAP(p.primitiveType, u16);

                read_buffer(at, p.vArray, p.formatCount, p.verts, p.formats);

                // WARNING: Endianess dependent read
                p.indexData = new u16[p.indices];
                memcpy(p.indexData, at, su16 * p.indices); at += su16 * p.indices;
                //for (u32 j{ 0 };j < p.indices;++j) {
                //    SWAP(p.indexData[j], u16);
                //}
                if (corrupt) { // Error Case
                    for (u32 j{ 0 };j < p.indices;++j) {
                        if (p.indexData[j] > p.verts) {
                            p.indexData[j] = (u16)p.verts - 1;
                        }
                    }
                }

                memcpy(&(p.usedBoneCount), at, 1); at += 1;
                assert(p.usedBoneCount <= MAX_BONES && ("Failed to load scene. Too many bones: " + i));

                p.usedBones = new u8[p.usedBoneCount];
                memcpy(p.usedBones, at, p.usedBoneCount); at += p.usedBoneCount;

                info.emplace_back(p);
            }
            return true;
        }

        bool read_buffer(const u8*& at, node& info) {
            u16 size{ 0 }; u32 i{ 0 };
            memcpy(&size, at, su16); at += su16;
            SWAP(size, u16);
            info.name.assign(at, at + size); at += size; // name

            for (i = 0;i < 3;++i) { // 3 rows 4 columns
                memcpy(&info.modeltm.x[i], at, su32); at += su32; // Pos
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

            info.isEnabled = NODE_ENABLED & info.nodeFlags;
            info.classID = (NODE_CLASS & info.nodeFlags);

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

        [[nodiscard]]
        std::vector<node> read_buffer(const u8*& at, mesh*& info, u32& count) {
            u32 j{ 0 }; node x;
            entityNodes.clear();

            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, x);
                // Assign Node Values
                info[i].name = x.name;
                info[i].modeltm = x.modeltm;
                info[i].nodeFlags = x.nodeFlags;
                info[i].id = x.id;
                info[i].parentIndex = x.parentIndex;
                info[i].childIndex = x.childIndex;
                info[i].isEnabled = x.isEnabled;
                info[i].classID = x.classID;

                info[i].index = i;
                entityNodes.push_back({ info[i].name, info[i].modeltm, info[i].nodeFlags, info[i].id, info[i].parentIndex, info[i].childIndex, info[i].isEnabled, info[i].classID, info[i].index });

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

            return entityNodes;
        }

        [[nodiscard]]
        std::vector<node> read_buffer(const u8*& at, camera*& info, u32& count) {
            node x;
            entityNodes.clear();

            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, x);
                // Assign Node Values
                info[i].name = x.name;
                info[i].modeltm = x.modeltm;
                info[i].nodeFlags = x.nodeFlags;
                info[i].id = x.id;
                info[i].parentIndex = x.parentIndex;
                info[i].childIndex = x.childIndex;
                info[i].isEnabled = x.isEnabled;
                info[i].classID = x.classID;

                info[i].index = i;
                entityNodes.push_back({ info[i].name, info[i].modeltm, info[i].nodeFlags, info[i].id, info[i].parentIndex, info[i].childIndex, info[i].isEnabled, info[i].classID, info[i].index });

                memcpy(&(info[i].front), at, su32); at += su32;
                SWAP(info[i].front, f32);
                memcpy(&(info[i].back), at, su32); at += su32;
                SWAP(info[i].back, f32);
                memcpy(&(info[i].FOV), at, su32); at += su32;
                SWAP(info[i].FOV, f32);
            }
            return entityNodes;
        }

        [[nodiscard]]
        std::vector<node> read_buffer(const u8*& at, light*& info, u32& count) {
            node x;
            entityNodes.clear();
            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, x);
                // Assign Node Values
                info[i].name = x.name;
                info[i].modeltm = x.modeltm;
                info[i].nodeFlags = x.nodeFlags;
                info[i].id = x.id;
                info[i].parentIndex = x.parentIndex;
                info[i].childIndex = x.childIndex;
                info[i].isEnabled = x.isEnabled;
                info[i].classID = x.classID;

                info[i].index = i;
                entityNodes.push_back({ info[i].name, info[i].modeltm, info[i].nodeFlags, info[i].id, info[i].parentIndex, info[i].childIndex, info[i].isEnabled, info[i].classID, info[i].index });

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
            return entityNodes;
        }

        [[nodiscard]]
        std::vector<node> read_buffer(const u8*& at, dummy*& info, u32& count) {
            u32 j{ 0 };
            node x;
            entityNodes.clear();
            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, x);
                // Assign Node Values
                info[i].name = x.name;
                info[i].modeltm = x.modeltm;
                info[i].nodeFlags = x.nodeFlags;
                info[i].id = x.id;
                info[i].parentIndex = x.parentIndex;
                info[i].childIndex = x.childIndex;
                info[i].isEnabled = x.isEnabled;
                info[i].classID = x.classID;

                info[i].index = i;
                entityNodes.push_back({ info[i].name, info[i].modeltm, info[i].nodeFlags, info[i].id, info[i].parentIndex, info[i].childIndex, info[i].isEnabled, info[i].classID, info[i].index });

                memcpy(&(info[i].boxMin.x), at, su32 * 3); at += su32 * 3;
                for (j = 0;j < 3;++j) {
                    SWAP(info[i].boxMin.x[j], f32);
                }
                memcpy(&(info[i].boxMax.x), at, su32 * 3); at += su32 * 3;
                for (j = 0;j < 3;++j) {
                    SWAP(info[i].boxMax.x[j], f32);
                }
            }
            return entityNodes;
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

        [[nodiscard]]
        std::vector<node> read_buffer(const u8*& at, shape*& info, u32& count) {
            node x;
            entityNodes.clear();
            s32 j{ 0 };
            for (u32 i{ 0 };i < count;++i) {
                read_buffer(at, x);
                // Assign Node Values
                info[i].name = x.name;
                info[i].modeltm = x.modeltm;
                info[i].nodeFlags = x.nodeFlags;
                info[i].id = x.id;
                info[i].parentIndex = x.parentIndex;
                info[i].childIndex = x.childIndex;
                info[i].isEnabled = x.isEnabled;
                info[i].classID = x.classID;

                info[i].index = i;
                entityNodes.push_back({ info[i].name, info[i].modeltm, info[i].nodeFlags, info[i].id, info[i].parentIndex, info[i].childIndex, info[i].isEnabled, info[i].classID, info[i].index });

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
            return entityNodes;
        }

        tools::math::float4 readFloat4(const u8*& at) {
            tools::math::float4 obj;

            memcpy(&(obj.x), at, su32); at += su32;
            obj.x = swap_endian<f32>(obj.x);

            memcpy(&(obj.y), at, su32); at += su32;
            obj.y = swap_endian<f32>(obj.y);

            memcpy(&(obj.z), at, su32); at += su32;
            obj.z = swap_endian<f32>(obj.z);

            memcpy(&(obj.w), at, su32); at += su32;
            obj.w = swap_endian<f32>(obj.w);

            return obj;
        }

        std::vector<tools::math::float4> 
        read_Float4Array16(const u8*& at, s32 count) {

            std::vector<tools::math::float4> out;
            out.reserve(count);

            if (count > 2) {
                tools::math::float4 minv = readFloat4(at);
                tools::math::float4 maxv = readFloat4(at);

                tools::math::float4 delta;
                delta.x = maxv.x - minv.x;
                delta.y = maxv.y - minv.y;
                delta.z = maxv.z - minv.z;
                delta.w = maxv.w - minv.w;

                tools::math::float4 zeta;
                for (int i = 0; i < count; ++i)
                {
                    u16 xi; float x;

                    // x
                    memcpy(&xi, at, su16); at += su16;

                    x = float(xi) * (1.f / 65535.f);
                    x *= delta.x;
                    x += minv.x;
                    zeta.x = x;

                    // y
                    memcpy(&xi, at, su16); at += su16;

                    x = float(xi) * (1.f / 65535.f);
                    x *= delta.y;
                    x += minv.y;
                    zeta.y = x;

                    // z
                    memcpy(&xi, at, su16); at += su16;

                    x = float(xi) * (1.f / 65535.f);
                    x *= delta.z;
                    x += minv.z;
                    zeta.z = x;

                    // w
                    memcpy(&xi, at, su16); at += su16;

                    x = float(xi) * (1.f / 65535.f);
                    x *= delta.w;
                    x += minv.w;
                    zeta.w = x;

                    out.emplace_back(zeta);
                }
            } else {
                for (int i = 0; i < count; ++i)
                    out.emplace_back(readFloat4(at));
            }

            return out;
        }

        tools::math::float3 readFloat3(const u8*& at) {
            tools::math::float3 obj;

            memcpy(&(obj.x[0]), at, su32); at += su32;
            obj.x[0] = swap_endian<f32>(obj.x[0]);

            memcpy(&(obj.x[1]), at, su32); at += su32;
            obj.x[1] = swap_endian<f32>(obj.x[1]);

            memcpy(&(obj.x[2]), at, su32); at += su32;
            obj.x[2] = swap_endian<f32>(obj.x[2]);

            return obj;
        }

        std::vector<tools::math::float4> 
        read_Float3Array16(const u8*& at, s32 count) {

            std::vector<tools::math::float4> out;
            out.reserve(count);

            if (count > 2)
            {
                tools::math::float3 minv = readFloat3(at);
                tools::math::float3 maxv = readFloat3(at);

                tools::math::float3 delta{};
                for (int k = 0; k < 3; ++k)
                    delta.x[k] = maxv.x[k] - minv.x[k];

                tools::math::float4 gamma{};
                tools::math::float3 zeta{};
                for (int i = 0; i < count; ++i)
                {
                    for (int k = 0; k < 3; ++k)
                    {
                        u16 xi;
                        memcpy(&xi, at, su16); at += su16;
                        float x = float(xi) * (1.f / 65535.f);
                        x *= delta.x[k];
                        x += minv.x[k];
                        zeta.x[k] = x;
                    }
                    gamma = zeta;
                    out.emplace_back(gamma);
                }
            }
            else
            {
                tools::math::float4 gamma{};
                for (int i = 0; i < count; ++i)
                    gamma = readFloat3(at);
                    out.emplace_back(gamma);
            }

            return out;
        }

        bool read_float3anim(const u8*& at, float3Animation& info) {
            memcpy(&(info.keyCount), at, su32); at += su32; info.keyCount = swap_endian<s32>(info.keyCount);

            info.keys = read_Float4Array16(at, info.keyCount);

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

                f32* keys = new f32[size];
                tools::math::float4 zeta{};

                memcpy(keys, at, size * length); at += size * length;
                for (u32 j{ 0 };j < size * length;++j) {
                    zeta.x = swap_endian<f32>(keys[j++]);
                    zeta.y = swap_endian<f32>(keys[j++]);
                    if(length>2) zeta.z = swap_endian<f32>(keys[j++]);
                    if(length>3) zeta.w = swap_endian<f32>(keys[j]);

                    info.keys.emplace_back(zeta); // is it supposed to be little or big endian?
                }
                delete[] keys;
                info.size = size;
            }
            else { // Implement in v193
                int dim;
                memcpy(&dim, at, su32); at += su32; dim = swap_endian<u32>(dim);
                assert((dim == 4 || dim == 3) && "Keyframe sequence in {0} dimension invalid ({1})");

                if (dim == 4) { // VertexFormat::DF_V4_32
                    info.dataFormat = "DF_V4_32"; // float32[4]
                    //obj = new KeyframeSequence(keys, VertexFormat::DF_V4_32);
                    //readFloat4Array16((float4*)obj->data(), keys);
                    info.keys = read_Float4Array16(at, info.keyCount); // Maybe Quaternion
                }
                else {
                    info.dataFormat = "DF_V3_32"; // float32[3]
                    //obj = new KeyframeSequence(keys, VertexFormat::DF_V3_32);
                    //readFloat3Array16((float3*)obj->data(), keys);
                    info.keys = read_Float3Array16(at, info.keyCount);
                }
            }

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
                if (version >= 192) memcpy(&info[i].isOptimized, at, 1); at += 1;

                if (!info[i].isOptimized) {
                    // not implementing rn

                    info[i].posKeyData_uo = new keyframeSequence();
                    info[i].rotKeyData = new keyframeSequence();
                    info[i].sclKeyData_uo = new keyframeSequence();

                    read_buffer(at, *info[i].posKeyData_uo);
                    read_buffer(at, *info[i].rotKeyData);
                    read_buffer(at, *info[i].sclKeyData_uo);
                }
                else { // New Implementation
                    info[i].posKeyData = new float3Animation();
                    info[i].rotKeyData = new keyframeSequence(); // Only this remains same
                    info[i].sclKeyData = new float3Animation();

                    read_float3anim(at, *info[i].posKeyData);
                    read_buffer(at, *info[i].rotKeyData);
                    read_float3anim(at, *info[i].sclKeyData);

                    info[i].endTime = 0.f;
                    if (version >= 193) {
                        memcpy(&(info[i].endTime), at, su32); at += su32;
                        info[i].endTime = swap_endian<f32>(info[i].endTime);
                    }
                    else {
                        info[i].endTime = float(info[i].rotKeyData->keyCount) * float(info[i].rotKeyRate);
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

        void find_children(std::vector<node>& lNodes) {
            int i{ 0 }; // if only i hadn't used an iterator
            for (auto lNode : lNodes) {
                ++i; if (lNode.parentIndex == 4294967295) continue;

                lNodes[lNode.parentIndex].childIndex.push_back(i);
            }
            return;
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

    TOOL_INTERFACE bool StoreData(const char* path, const char* texpath, const char* outpath) {

        std::vector<node> hgrNodes;
        { // File Test
            std::string file = path;
            file = file.substr(file.find_last_of("\\") + 1, file.length() - file.find_last_of("\\") - 5);
            if (file._Equal("hypno_level01")) corrupt = true;
            if (file._Equal("hypno_level02")) corrupt = true;
            if (file._Equal("hypno_level03")) corrupt = true;
            if (file._Equal("hypno_level04")) corrupt = true;
            if (file._Equal("mushroom_level01")) corrupt = true;
            if (file._Equal("mushroom_level02")) corrupt = true;
            if (file._Equal("mushroom_level03")) corrupt = true;
            if (file._Equal("mushroom_level04")) corrupt = true;
            if (file._Equal("score_level01")) corrupt = true;
            if (file._Equal("score_level02")) corrupt = true;
            if (file._Equal("score_level03")) corrupt = true;
            if (file._Equal("skybean_level02")) corrupt = true;
            if (file._Equal("skybean_level03")) corrupt = true;
            if (file._Equal("skybean_level04")) corrupt = true;
            if (file._Equal("worldmap")) corrupt = true;
        }

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

        // TODO: replace array pointers with vector

        memcpy(&(entityInfo.Texture_Count), at, su32); at += su32;
        entityInfo.Texture_Count = swap_endian<u32>(entityInfo.Texture_Count);
        std::vector<texture_info> Textures;
        read_buffer(at, Textures, entityInfo.Texture_Count);

        memcpy(&(entityInfo.Material_Count), at, su32); at += su32;
        entityInfo.Material_Count = swap_endian<u32>(entityInfo.Material_Count);
        std::vector<material_info> Materials;
        read_buffer(at, Materials, entityInfo.Material_Count);

        check_id(at, *header);

        memcpy(&(entityInfo.Primitive_Count), at, su32); at += su32;
        entityInfo.Primitive_Count = swap_endian<u32>(entityInfo.Primitive_Count);
        std::vector<primitive_info> Primitives;
        read_buffer(at, Primitives, entityInfo.Primitive_Count);

        check_id(at, *header);

        memcpy(&(entityInfo.Mesh_Count), at, su32); at += su32;
        entityInfo.Mesh_Count = swap_endian<u32>(entityInfo.Mesh_Count);
        mesh* Meshes = new mesh[entityInfo.Mesh_Count];
        auto x = read_buffer(at, Meshes, entityInfo.Mesh_Count);

        hgrNodes.insert(hgrNodes.end(),
            std::make_move_iterator(x.begin()),
            std::make_move_iterator(x.end()));

        check_id(at, *header);

        memcpy(&(entityInfo.Camera_Count), at, su32); at += su32;
        entityInfo.Camera_Count = swap_endian<u32>(entityInfo.Camera_Count);
        camera* Cameras = new camera[entityInfo.Camera_Count];
        if (entityInfo.Camera_Count > 0) {
            x = read_buffer(at, Cameras, entityInfo.Camera_Count);
            hgrNodes.insert(hgrNodes.end(),
                std::make_move_iterator(x.begin()),
                std::make_move_iterator(x.end()));
        }

        check_id(at, *header);

        memcpy(&(entityInfo.Light_Count), at, su32); at += su32;
        entityInfo.Light_Count = swap_endian<u32>(entityInfo.Light_Count);
        light* Lights = new light[entityInfo.Light_Count];
        if (entityInfo.Light_Count > 0) {
            x = read_buffer(at, Lights, entityInfo.Light_Count);
            hgrNodes.insert(hgrNodes.end(),
                std::make_move_iterator(x.begin()),
                std::make_move_iterator(x.end()));
        }

        check_id(at, *header);

        memcpy(&(entityInfo.Dummy_Count), at, su32); at += su32;
        entityInfo.Dummy_Count = swap_endian<u32>(entityInfo.Dummy_Count);
        dummy* Dummies = new dummy[entityInfo.Dummy_Count];
        if (entityInfo.Dummy_Count > 0) {
            x = read_buffer(at, Dummies, entityInfo.Dummy_Count);
            hgrNodes.insert(hgrNodes.end(),
                std::make_move_iterator(x.begin()),
                std::make_move_iterator(x.end()));
        }

        check_id(at, *header);

        memcpy(&(entityInfo.Shape_Count), at, su32); at += su32;
        entityInfo.Shape_Count = swap_endian<u32>(entityInfo.Shape_Count);
        shape* Shapes = new shape[entityInfo.Shape_Count];
        if (entityInfo.Shape_Count > 0) {
            x = read_buffer(at, Shapes, entityInfo.Shape_Count);
            hgrNodes.insert(hgrNodes.end(),
                std::make_move_iterator(x.begin()),
                std::make_move_iterator(x.end()));
        }

        check_id(at, *header);

        memcpy(&(entityInfo.OtherNodes_Count), at, su32); at += su32;
        entityInfo.OtherNodes_Count = swap_endian<u32>(entityInfo.OtherNodes_Count);
        node* otherNodes = new node[entityInfo.OtherNodes_Count];
        if (entityInfo.OtherNodes_Count > 0) {
            for (u32 i{ 0 };i < entityInfo.OtherNodes_Count; ++i) {
                read_buffer(at, otherNodes[i]);
                hgrNodes.push_back(otherNodes[i]);
            }
        }

        check_id(at, *header);

        memcpy(&(entityInfo.TransformAnimation_Count), at, su32); at += su32;
        entityInfo.TransformAnimation_Count = swap_endian<u32>(entityInfo.TransformAnimation_Count);
        transformAnimation* TransformAnimations = new transformAnimation[entityInfo.TransformAnimation_Count];
        if (entityInfo.TransformAnimation_Count > 0) {
            read_buffer(at, TransformAnimations, entityInfo.TransformAnimation_Count);
        }
        
        check_id(at, *header);
        
        memcpy(&(entityInfo.UserProperties_Count), at, su32); at += su32;
        entityInfo.UserProperties_Count = swap_endian<u32>(entityInfo.UserProperties_Count);
        userProperty* UserProperties  = new userProperty[entityInfo.UserProperties_Count];
        if (entityInfo.UserProperties_Count > 0) {
            read_buffer(at, UserProperties, entityInfo.UserProperties_Count);
        }

        // Check if all the data is read:
        assert(at == (buffer.get() + size));

        assetData Asset{};
        // Fill Data
        Asset.info = header;
        Asset.scene_param = sceneParams;
        Asset.entityInfo = &entityInfo;
        Asset.texInfo = Textures;
        Asset.matInfo = Materials;
        Asset.primInfo = Primitives;

        find_children(hgrNodes); // Should I remove it? I'm not using it rn...
        Asset.Nodes = hgrNodes;

        Asset.meshInfo = Meshes;
        Asset.cameraInfo = Cameras;
        Asset.lightInfo = Lights;
        Asset.dummyInfo = Dummies;
        Asset.shapeinfo = Shapes;
        Asset.otherNodeInfo = otherNodes;

        Asset.transAnim = TransformAnimations;
        Asset.userProp = UserProperties;

        // TODO:
        // connect bones
        // connect lights to Meshes

        CreateFBX(Asset, path, texpath, outpath); // FBX Exporter

        // to avoid memory leaks
        u32 i{ 0 };
        {
            delete header;
            delete sceneParams;
            Textures.clear();
            for (i = 0;i < entityInfo.Material_Count;++i) {
                delete[] Materials[i].TexParams;
                delete[] Materials[i].Vec4Params;
                delete[] Materials[i].FloatParams;
            }
            for (i = 0;i < entityInfo.Primitive_Count;++i) {
                delete[] Primitives[i].formats;
                for (u32 j{ 0 };j < Primitives[i].formatCount;++j) {
                    delete[] Primitives[i].vArray[j].value;
                }
                delete[] Primitives[i].vArray;
                delete[] Primitives[i].indexData;
                delete[] Primitives[i].usedBones;
            }
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
            delete[] otherNodes;
            for (i = 0;i < entityInfo.TransformAnimation_Count;++i) {
                if (!(TransformAnimations[i].isOptimized)) {
                    delete TransformAnimations[i].posKeyData_uo;
                    delete TransformAnimations[i].rotKeyData;
                    delete TransformAnimations[i].sclKeyData_uo;
                }
                else {
                    delete TransformAnimations[i].posKeyData;
                    delete TransformAnimations[i].rotKeyData;
                    delete TransformAnimations[i].sclKeyData;
                }
            }
            delete[] TransformAnimations;
            delete[] UserProperties;
        }

        {
            entityNodes.clear();
            entityInfo.Texture_Count = 0;
            entityInfo.Material_Count = 0;
            entityInfo.Primitive_Count = 0;
            entityInfo.Mesh_Count = 0;
            entityInfo.Camera_Count = 0;
            entityInfo.Light_Count = 0;
            entityInfo.Dummy_Count = 0;
            entityInfo.Shape_Count = 0;
            entityInfo.OtherNodes_Count = 0;
            entityInfo.TransformAnimation_Count = 0;
            entityInfo.UserProperties_Count = 0;
        }

        return true;
    }

    // Implement Later
    /*
    // connect bones
	n = 0;
	for ( int i = 0 ; i < meshbonecounts.size() ; ++i )
	{
		const MeshBoneCount& mbc = meshbonecounts[i];
		int bonecount = mbc.bonecount;
		for ( int k = 0 ; k < bonecount ; ++k )
		{
			const MeshBone& mb = meshbones[n++];
			int ix = mb.boneindex;
			if ( ix < 0 || ix >= nodes.size() )
				throwError( IOException( Format("Failed to load scene \"{0}\". Invalid bone index ({1}) in \"{2}\".", filename, ix, mbc.mesh->name()) ) );

			mbc.mesh->addBone( nodes[ix], mb.invresttm );
		}
	}
	assert( meshbones.size() == n );

	// connect lights to meshes
	LightSorter lightsorter;
	lightsorter.collectLights( this );
	if ( lightsorter.lights() > 0 )
	{
		for ( int i = 0 ; i < meshbonecounts.size() ; ++i )
		{
			Mesh* mesh = meshbonecounts[i].mesh;

			float3 meshpos = mesh->worldTransform().transform( (mesh->boundBoxMax()+mesh->boundBoxMin())*.5f );
			Array<Light*>& lights = lightsorter.getLightsByDistance( meshpos );

			if ( lights.size() > 0 )
				mesh->addLight( lights[0] );
		}
	}

	// create particle systems based on user properties Particle=<name>
	if ( m_userProperties != 0 )
	{
		PropertyParser parser;
		for ( HashtableIterator<String,String> it = m_userProperties->begin() ; it != m_userProperties->end() ; ++it )
		{
			parser.reset( it.value(), it.key() );

			float time = 0.f;
			for ( PropertyParser::ConstIterator i = parser.begin() ; i != parser.end() ; ++i )
			{
				if ( !strcmp(i.key(),"perspectivecorrection") )
				{
					float perspf = 5.f;
					if ( sscanf(i.value(),"%g",&perspf) != 1 )
						throwError( IOException( Format("Failed parse PerspectiveCorrection=<level 0-10> User Property field from object \"{0}\" in scene \"{1}\"", it.key(), name()) ) );
					int persp = (int)perspf;
					if ( persp < 0 || persp > 10 )
						throwError( IOException( Format("Failed parse PerspectiveCorrection=<level 0-10> User Property field from object \"{0}\" in scene \"{1}\"", it.key(), name()) ) );
					
					Node* node = getNodeByName( it.key() );
					if ( node->classId() == Node::NODE_MESH )
					{
						Mesh* mesh = static_cast<Mesh*>( node );
						for ( int i = 0 ; i < mesh->primitives() ; ++i )
							mesh->getPrimitive(i)->setPerspectiveCorrection( persp );
					}
				}
				else if ( !strcmp(i.key(),"time") )
				{
					if ( sscanf(i.value(),"%g",&time) != 1 )
						throwError( IOException( Format("Failed parse Time=<seconds> User Property field from object \"{0}\" in scene \"{1}\"", it.key(), name()) ) );
				}
#ifndef HGR_NOPARTICLES
				else if ( !strcmp(i.key(),"particle") )
				{
					PathName particlepathname( particlepathsz, i.value() );

					// append .prs extension
					buf.resize( strlen(particlepathname.toString()) + 1 );
					strcpy( buf.begin(), particlepathname.toString() );
					buf.resize( buf.size()-1 );
					buf.add( '.' );
					buf.add( 'p' );
					buf.add( 'r' );
					buf.add( 's' );
					buf.add( 0 );

					// create particle system from file <particlename>
					P(ParticleSystem) particle = res->getParticleSystem( buf.begin(), texturepath, shaderpath );

					// set particle instance specific properties
					particle->setDelay( time );

					// link particle to node
					Node* node = getNodeByName( it.key() );
					particle->linkTo( node );
				}
#endif // HGR_NOPARTICLES
			}
		}
	}
    */
}