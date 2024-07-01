using System;
using System.Collections.Generic;
using Assimp;
using Assimp.Unmanaged;

namespace KA3D_Tools.AssimpC
{
    public sealed class Face : IMarshalable<Face, AiFace>
    {
        private List<int> m_indices;

        /// <summary>
        /// Gets the number of indices defined in the face.
        /// </summary>
        public int IndexCount
        {
            get
            {
                return m_indices.Count;
            }
        }

        /// <summary>
        /// Gets if the face has faces (should always be true).
        /// </summary>
        public bool HasIndices
        {
            get
            {
                return m_indices.Count > 0;
            }
        }

        /// <summary>
        /// Gets or sets the indices that refer to positions of vertex data in the mesh's vertex 
        /// arrays.
        /// </summary>
        public List<int> Indices
        {
            get
            {
                return m_indices;
            }
        }

        /// <summary>
        /// Constructs a new instance of the <see cref="Face"/> class.
        /// </summary>
        public Face()
        {
            m_indices = new List<int>();
        }

        /// <summary>
        /// Constructs a new instance of the <see cref="Face"/> class.
        /// </summary>
        /// <param name="indices">Face indices</param>
        public Face(int[] indices)
        {
            m_indices = new List<int>();

            if (indices != null)
                m_indices.AddRange(indices);
        }

        #region IMarshalable Implementation

        /// <summary>
        /// Gets if the native value type is blittable (that is, does not require marshaling by the runtime, e.g. has MarshalAs attributes).
        /// </summary>
        bool IMarshalable<Face, AiFace>.IsNativeBlittable
        {
            get { return true; }
        }

        /// <summary>
        /// Writes the managed data to the native value.
        /// </summary>
        /// <param name="thisPtr">Optional pointer to the memory that will hold the native value.</param>
        /// <param name="nativeValue">Output native value</param>
        void IMarshalable<Face, AiFace>.ToNative(IntPtr thisPtr, out AiFace nativeValue)
        {
            nativeValue.NumIndices = (uint)IndexCount;
            nativeValue.Indices = IntPtr.Zero;

            if (nativeValue.NumIndices > 0)
                nativeValue.Indices = MemoryHelper.ToNativeArray<int>(m_indices.ToArray());
        }

        /// <summary>
        /// Frees unmanaged memory created by <see cref="IMarshalable{Face, AiFace}.ToNative"/>.
        /// </summary>
        /// <param name="nativeValue">Native value to free</param>
        /// <param name="freeNative">True if the unmanaged memory should be freed, false otherwise.</param>
        public static void FreeNative(IntPtr nativeValue, bool freeNative)
        {
            if (nativeValue == IntPtr.Zero)
                return;

            AiFace aiFace = MemoryHelper.Read<AiFace>(nativeValue);

            if (aiFace.NumIndices > 0 && aiFace.Indices != IntPtr.Zero)
                MemoryHelper.FreeMemory(aiFace.Indices);

            if (freeNative)
                MemoryHelper.FreeMemory(nativeValue);
        }

        public void FromNative(in AiFace nativeValue)
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}
