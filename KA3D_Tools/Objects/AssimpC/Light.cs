﻿using System;
using Assimp;
using Assimp.Unmanaged;

namespace KA3D_Tools.AssimpC
{
    public sealed class Light : IMarshalable<Light, AiLight>
    {
        private String m_name;
        private LightSourceType m_lightType;
        private float m_angleInnerCone;
        private float m_angleOuterCone;
        private float m_attConstant;
        private float m_attLinear;
        private float m_attQuadratic;
        private Vector3D m_position;
        private Vector3D m_direction;
        private Vector3D m_up;
        private Vector2D m_size;
        private Color3D m_diffuse;
        private Color3D m_specular;
        private Color3D m_ambient;


        /// <summary>
        /// Gets or sets the name of the light source. This corresponds to a node present in the scenegraph.
        /// </summary>
        public String Name
        {
            get
            {
                return m_name;
            }
            set
            {
                m_name = value;
            }
        }

        /// <summary>
        /// Gets or sets the type of light source. This should never be undefined.
        /// </summary>
        public LightSourceType LightType
        {
            get
            {
                return m_lightType;
            }
            set
            {
                m_lightType = value;
            }
        }

        /// <summary>
        /// Gets or sets the inner angle of a spot light's light cone. The spot light has
        /// maximum influence on objects inside this angle. The angle is given in radians, it
        /// is 2PI for point lights and defined for directional lights.
        /// </summary>
        public float AngleInnerCone
        {
            get
            {
                return m_angleInnerCone;
            }
            set
            {
                m_angleInnerCone = value;
            }
        }

        /// <summary>
        /// Gets or sets the outer angle of a spot light's light cone. The spot light does not affect objects outside
        /// this angle. The angle is given in radians. It is 2PI for point lights and undefined for
        /// directional lights. The outer angle must be greater than or equal to the inner angle.
        /// </summary>
        public float AngleOuterCone
        {
            get
            {
                return m_angleOuterCone;
            }
            set
            {
                m_angleOuterCone = value;
            }
        }

        /// <summary>
        /// Gets or sets the constant light attenuation factor. The intensity of the light source
        /// at a given distance 'd' from the light position is <code>Atten = 1 / (att0 + att1 * d + att2 * d*d)</code>.
        /// <para>This member corresponds to the att0 variable in the equation and is undefined for directional lights.</para>
        /// </summary>
        public float AttenuationConstant
        {
            get
            {
                return m_attConstant;
            }
            set
            {
                m_attConstant = value;
            }
        }

        /// <summary>
        /// Gets or sets the linear light attenuation factor. The intensity of the light source
        /// at a given distance 'd' from the light position is <code>Atten = 1 / (att0 + att1 * d + att2 * d*d)</code>
        /// <para>This member corresponds to the att1 variable in the equation and is undefined for directional lights.</para>
        /// </summary>
        public float AttenuationLinear
        {
            get
            {
                return m_attLinear;
            }
            set
            {
                m_attLinear = value;
            }
        }

        /// <summary>
        /// Gets or sets the quadratic light attenuation factor. The intensity of the light source
        /// at a given distance 'd' from the light position is <code>Atten = 1 / (att0 + att1 * d + att2 * d*d)</code>.
        /// <para>This member corresponds to the att2 variable in the equation and is undefined for directional lights.</para>
        /// </summary>
        public float AttenuationQuadratic
        {
            get
            {
                return m_attQuadratic;
            }
            set
            {
                m_attQuadratic = value;
            }
        }

        /// <summary>
        /// Gets or sets the position of the light source in space, relative to the
        /// transformation of the node corresponding to the light. This is undefined for
        /// directional lights.
        /// </summary>
        public Vector3D Position
        {
            get
            {
                return m_position;
            }
            set
            {
                m_position = value;
            }
        }

        /// <summary>
        /// Gets or sets the direction of the light source in space, relative to the transformation
        /// of the node corresponding to the light. This is undefined for point lights.
        /// </summary>
        public Vector3D Direction
        {
            get
            {
                return m_direction;
            }
            set
            {
                m_direction = value;
            }
        }

        /// <summary>
        /// Gets or sets the direction up.
        /// </summary>
        public Vector3D Up
        {
            get
            {
                return m_up;
            }
            set
            {
                m_up = value;
            }
        }

        /// <summary>
        /// Gets or sets size of na Area light.
        /// </summary>
        public Vector2D Size
        {
            get
            {
                return m_size;
            }
            set
            {
                m_size = value;
            }
        }

        /// <summary>
        /// Gets or sets the diffuse color of the light source.  The diffuse light color is multiplied with
        /// the diffuse material color to obtain the final color that contributes to the diffuse shading term.
        /// </summary>
        public Color3D ColorDiffuse
        {
            get
            {
                return m_diffuse;
            }
            set
            {
                m_diffuse = value;
            }
        }

        /// <summary>
        /// Gets or sets the specular color of the light source. The specular light color is multiplied with the
        /// specular material color to obtain the final color that contributes to the specular shading term.
        /// </summary>
        public Color3D ColorSpecular
        {
            get
            {
                return m_specular;
            }
            set
            {
                m_specular = value;
            }
        }

        /// <summary>
        /// Gets or sets the ambient color of the light source. The ambient light color is multiplied with the ambient
        /// material color to obtain the final color that contributes to the ambient shading term.
        /// </summary>
        public Color3D ColorAmbient
        {
            get
            {
                return m_ambient;
            }
            set
            {
                m_ambient = value;
            }
        }

        /// <summary>
        /// Constructs a new instance of the <see cref="Light"/> class.
        /// </summary>
        public Light()
        {
            m_lightType = LightSourceType.Undefined;
        }

        #region IMarshalable Implementation

        /// <summary>
        /// Gets if the native value type is blittable (that is, does not require marshaling by the runtime, e.g. has MarshalAs attributes).
        /// </summary>
        bool IMarshalable<Light, AiLight>.IsNativeBlittable
        {
            get { return true; }
        }

        /// <summary>
        /// Writes the managed data to the native value.
        /// </summary>
        /// <param name="thisPtr">Optional pointer to the memory that will hold the native value.</param>
        /// <param name="nativeValue">Output native value</param>
        void IMarshalable<Light, AiLight>.ToNative(IntPtr thisPtr, out AiLight nativeValue)
        {
            nativeValue.Name = new AiString(m_name);
            nativeValue.Type = m_lightType;
            nativeValue.AngleInnerCone = m_angleInnerCone;
            nativeValue.AngleOuterCone = m_angleOuterCone;
            nativeValue.AttenuationConstant = m_attConstant;
            nativeValue.AttenuationLinear = m_attLinear;
            nativeValue.AttenuationQuadratic = m_attQuadratic;
            nativeValue.ColorAmbient = m_ambient;
            nativeValue.ColorDiffuse = m_diffuse;
            nativeValue.ColorSpecular = m_specular;
            nativeValue.Direction = m_direction;
            nativeValue.Position = m_position;
            nativeValue.Up = m_up;
            nativeValue.AreaSize = m_size;
        }

        /// <summary>
        /// Frees unmanaged memory created by <see cref="IMarshalable{Light, AiLight}.ToNative"/>.
        /// </summary>
        /// <param name="nativeValue">Native value to free</param>
        /// <param name="freeNative">True if the unmanaged memory should be freed, false otherwise.</param>
        public static void FreeNative(IntPtr nativeValue, bool freeNative)
        {
            if (nativeValue != IntPtr.Zero && freeNative)
                MemoryHelper.FreeMemory(nativeValue);
        }

        public void FromNative(in AiLight nativeValue)
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}
