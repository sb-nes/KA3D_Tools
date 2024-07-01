using System;
using System.Collections.Generic;
using Assimp;
using Assimp.Unmanaged;

namespace KA3D_Tools.AssimpC
{
    /// <summary>
    /// This part of code is a modified version of assimp-net library
    /// </summary>

    public sealed class Scene : IMarshalable<Scene, AiScene>
    {
        private SceneFlags m_flags;
        private Node m_rootNode;
        private List<Mesh> m_meshes;
        private List<Light> m_lights;
        private List<Camera> m_cameras;
        private List<EmbeddedTexture> m_textures;
        private List<Animation> m_animations;
        private List<Material> m_materials;

        /// <summary>
        /// Gets or sets the state of the imported scene. By default no flags are set, but
        /// issues can arise if the flag is set to incomplete.
        /// </summary>
        public SceneFlags SceneFlags
        {
            get
            {
                return m_flags;
            }
            set
            {
                m_flags = value;
            }
        }

        /// <summary>
        /// Gets or sets the root node of the scene graph. There will always be at least the root node
        /// if the import was successful and no special flags have been set. Presence of further nodes
        /// depends on the format and content of the imported file.
        /// </summary>
        public Node RootNode
        {
            get
            {
                return m_rootNode;
            }
            set
            {
                m_rootNode = value;
            }
        }

        /// <summary>
        /// Gets if the scene contains meshes. Unless if no special scene flags are set
        /// this should always be true.
        /// </summary>
        public bool HasMeshes
        {
            get
            {
                return m_meshes.Count > 0;
            }
        }

        /// <summary>
        /// Gets the number of meshes in the scene.
        /// </summary>
        public int MeshCount
        {
            get
            {
                return m_meshes.Count;
            }
        }

        /// <summary>
        /// Gets the meshes contained in the scene, if any.
        /// </summary>
        public List<Mesh> Meshes
        {
            get
            {
                return m_meshes;
            }
            set
            {
                m_meshes = value;
            }
        }


        /// <summary>
        /// Gets if the scene contains any lights.
        /// </summary>
        public bool HasLights
        {
            get
            {
                return m_lights.Count > 0;
            }
        }

        /// <summary>
        /// Gets the number of lights in the scene.
        /// </summary>
        public int LightCount
        {
            get
            {
                return m_lights.Count;
            }
        }

        /// <summary>
        /// Gets the lights in the scene, if any.
        /// </summary>
        public List<Light> Lights
        {
            get
            {
                return m_lights;
            }
            set
            {
                m_lights = value;
            }
        }

        /// <summary>
        /// Gets if the scene contains any cameras.
        /// </summary>
        public bool HasCameras
        {
            get
            {
                return m_cameras.Count > 0;
            }
        }

        /// <summary>
        /// Gets the number of cameras in the scene.
        /// </summary>
        public int CameraCount
        {
            get
            {
                return m_cameras.Count;
            }
        }
        /// <summary>
        /// Gets the cameras in the scene, if any.
        /// </summary>
        public List<Camera> Cameras
        {
            get
            {
                return m_cameras;
            }
            set
            {
                m_cameras = value;
            }
        }

        /// <summary>
        /// Gets if the scene contains embedded textures.
        /// </summary>
        public bool HasTextures
        {
            get
            {
                return m_textures.Count > 0;
            }
        }

        /// <summary>
        /// Gets the number of embedded textures in the scene.
        /// </summary>
        public int TextureCount
        {
            get
            {
                return m_textures.Count;
            }
        }

        /// <summary>
        /// Gets the embedded textures in the scene, if any.
        /// </summary>
        public List<EmbeddedTexture> Textures
        {
            get
            {
                return m_textures;
            }
            set
            {
                m_textures = value;
            }
        }

        /// <summary>
        /// Gets if the scene contains any animations.
        /// </summary>
        public bool HasAnimations
        {
            get
            {
                return m_animations.Count > 0;
            }
        }

        /// <summary>
        /// Gets the number of animations in the scene.
        /// </summary>
        public int AnimationCount
        {
            get
            {
                return m_animations.Count;
            }
        }

        /// <summary>
        /// Gets the animations in the scene, if any.
        /// </summary>
        public List<Animation> Animations
        {
            get
            {
                return m_animations;
            }
            set
            {
                m_animations = value;
            }
        }

        /// <summary>
        /// Gets if the scene contains any materials. There should always be at least the
        /// default Assimp material if no materials were loaded.
        /// </summary>
        public bool HasMaterials
        {
            get
            {
                return m_materials.Count > 0;
            }
        }

        /// <summary>
        /// Gets the number of materials in the scene. There should always be at least the
        /// default Assimp material if no materials were loaded.
        /// </summary>
        public int MaterialCount
        {
            get
            {
                return m_materials.Count;
            }
        }

        /// <summary>
        /// Gets the materials in the scene.
        /// </summary>
        public List<Material> Materials
        {
            get
            {
                return m_materials;
            }
            set
            {
                m_materials = value;
            }
        }

        /// <summary>
        /// Constructs a new instance of the <see cref="Scene"/> class.
        /// </summary>
        public Scene()
        {
            m_flags = SceneFlags.None;
            m_rootNode = null;
            m_meshes = new List<Mesh>();
            m_lights = new List<Light>();
            m_cameras = new List<Camera>();
            m_textures = new List<EmbeddedTexture>();
            m_animations = new List<Animation>();
            m_materials = new List<Material>();
        }

        /// <summary>
        /// Clears the scene of all components.
        /// </summary>
        public void Clear()
        {
            m_rootNode = null;
            m_meshes.Clear();
            m_lights.Clear();
            m_cameras.Clear();
            m_textures.Clear();
            m_animations.Clear();
            m_materials.Clear();
        }

        /// <summary>
        /// Marshals a managed scene to unmanaged memory. The unmanaged memory must be freed with a call to
        /// <see cref="FreeUnmanagedScene"/>, the memory is owned by AssimpNet and cannot be freed by the native library.
        /// </summary>
        /// <param name="scene">Scene data</param>
        /// <returns>Unmanaged scene or NULL if the scene is null.</returns>
        public static IntPtr ToUnmanagedScene(Scene scene)
        {
            if (scene == null)
                return IntPtr.Zero;

            return MemoryHelper.ToNativePointer<Scene, AiScene>(scene);
        }

        /// <summary>
        /// Marshals an unmanaged scene to managed memory. This does not free the unmanaged memory.
        /// </summary>
        /// <param name="scenePtr">The unmanaged scene data</param>
        /// <returns>The managed scene, or null if the pointer is NULL</returns>
        public static Scene FromUnmanagedScene(IntPtr scenePtr)
        {
            if (scenePtr == IntPtr.Zero)
                return null;

            return MemoryHelper.FromNativePointer<Scene, AiScene>(scenePtr);
        }

        /// <summary>
        /// Frees unmanaged memory allocated -ONLY- in <see cref="ToUnmanagedScene"/>. To free an unmanaged scene allocated by the unmanaged Assimp library,
        /// call the appropiate <see cref="AssimpLibrary.ReleaseImport"/> function.
        /// </summary>
        /// <param name="scenePtr">Pointer to unmanaged scene data.</param>
        public static void FreeUnmanagedScene(IntPtr scenePtr)
        {
            if (scenePtr == IntPtr.Zero)
                return;

            FreeNative(scenePtr, true);
        }

        #region IMarshalable Implementation

        /// <summary>
        /// Gets if the native value type is blittable (that is, does not require marshaling by the runtime, e.g. has MarshalAs attributes).
        /// </summary>
        bool IMarshalable<Scene, AiScene>.IsNativeBlittable
        {
            get { return true; }
        }

        /// <summary>
        /// Writes the managed data to the native value.
        /// </summary>
        /// <param name="thisPtr">Optional pointer to the memory that will hold the native value.</param>
        /// <param name="nativeValue">Output native value</param>
        void IMarshalable<Scene, AiScene>.ToNative(IntPtr thisPtr, out AiScene nativeValue)
        {
            nativeValue.Flags = m_flags;
            nativeValue.Materials = IntPtr.Zero;
            nativeValue.RootNode = IntPtr.Zero;
            nativeValue.Meshes = IntPtr.Zero;
            nativeValue.Lights = IntPtr.Zero;
            nativeValue.Cameras = IntPtr.Zero;
            nativeValue.Textures = IntPtr.Zero;
            nativeValue.Animations = IntPtr.Zero;
            nativeValue.PrivateData = IntPtr.Zero;

            nativeValue.NumMaterials = (uint)MaterialCount;
            nativeValue.NumMeshes = (uint)MeshCount;
            nativeValue.NumLights = (uint)LightCount;
            nativeValue.NumCameras = (uint)CameraCount;
            nativeValue.NumTextures = (uint)TextureCount;
            nativeValue.NumAnimations = (uint)AnimationCount;

            //Write materials
            if (nativeValue.NumMaterials > 0)
                nativeValue.Materials = MemoryHelper.ToNativeArray<Material, AiMaterial>(m_materials.ToArray(), true);

            //Write scenegraph
            if (m_rootNode != null)
                nativeValue.RootNode = MemoryHelper.ToNativePointer<Node, AiNode>(m_rootNode);

            //Write meshes
            if (nativeValue.NumMeshes > 0)
                nativeValue.Meshes = MemoryHelper.ToNativeArray<Mesh, AiMesh>(m_meshes.ToArray(), true);

            //Write lights
            if (nativeValue.NumLights > 0)
                nativeValue.Lights = MemoryHelper.ToNativeArray<Light, AiLight>(m_lights.ToArray(), true);

            //Write cameras
            if (nativeValue.NumCameras > 0)
                nativeValue.Cameras = MemoryHelper.ToNativeArray<Camera, AiCamera>(m_cameras.ToArray(), true);

            //Write textures
            if (nativeValue.NumTextures > 0)
                nativeValue.Textures = MemoryHelper.ToNativeArray<EmbeddedTexture, AiTexture>(m_textures.ToArray(), true);

            //Write animations
            if (nativeValue.NumAnimations > 0)
                nativeValue.Animations = MemoryHelper.ToNativeArray<Animation, AiAnimation>(m_animations.ToArray(), true);
        }

        /// <summary>
        /// Frees unmanaged memory created by <see cref="IMarshalable{Scene, AiScene}.ToNative"/>.
        /// </summary>
        /// <param name="nativeValue">Native value to free</param>
        /// <param name="freeNative">True if the unmanaged memory should be freed, false otherwise.</param>
        public static void FreeNative(IntPtr nativeValue, bool freeNative)
        {
            if (nativeValue == IntPtr.Zero)
                return;

            AiScene aiScene = MemoryHelper.Read<AiScene>(nativeValue);

            if (aiScene.NumMaterials > 0 && aiScene.Materials != IntPtr.Zero)
                MemoryHelper.FreeNativeArray<AiMaterial>(aiScene.Materials, (int)aiScene.NumMaterials, Material.FreeNative, true);

            if (aiScene.RootNode != IntPtr.Zero)
                Node.FreeNative(aiScene.RootNode, true);

            if (aiScene.NumMeshes > 0 && aiScene.Meshes != IntPtr.Zero)
                MemoryHelper.FreeNativeArray<AiMesh>(aiScene.Meshes, (int)aiScene.NumMeshes, Mesh.FreeNative, true);

            if (aiScene.NumLights > 0 && aiScene.Lights != IntPtr.Zero)
                MemoryHelper.FreeNativeArray<AiLight>(aiScene.Lights, (int)aiScene.NumLights, Light.FreeNative, true);

            if (aiScene.NumCameras > 0 && aiScene.Cameras != IntPtr.Zero)
                MemoryHelper.FreeNativeArray<AiCamera>(aiScene.Cameras, (int)aiScene.NumCameras, Camera.FreeNative, true);

            if (aiScene.NumTextures > 0 && aiScene.Textures != IntPtr.Zero)
                MemoryHelper.FreeNativeArray<AiTexture>(aiScene.Textures, (int)aiScene.NumTextures, EmbeddedTexture.FreeNative, true);

            if (aiScene.NumAnimations > 0 && aiScene.Animations != IntPtr.Zero)
                MemoryHelper.FreeNativeArray<AiAnimation>(aiScene.Animations, (int)aiScene.NumAnimations, Animation.FreeNative, true);

            if (freeNative)
                MemoryHelper.FreeMemory(nativeValue);
        }
        public void FromNative(in AiScene nativeValue)
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}
