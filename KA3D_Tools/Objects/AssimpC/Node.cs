﻿using System;
using System.Collections.Generic;
using Assimp;
using Assimp.Unmanaged;

namespace KA3D_Tools.AssimpC
{
    /// <summary>
    /// A node in the imported model hierarchy.
    /// </summary>
    public sealed class Node : IMarshalable<Node, AiNode>
    {
        private String m_name;
        private Matrix4x4 m_transform;
        private Node m_parent;
        private NodeCollection m_children;
        private List<int> m_meshes;
        private Metadata m_metaData;

        /// <summary>
        /// Gets or sets the name of the node.
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
        /// Gets or sets the transformation of the node relative to its parent.
        /// </summary>
        public Matrix4x4 Transform
        {
            get
            {
                return m_transform;
            }
            set
            {
                m_transform = value;
            }
        }

        /// <summary>
        /// Gets the node's parent, if it exists. 
        /// </summary>
        public Node Parent
        {
            get
            {
                return m_parent;
            }
            set
            {
                m_parent = value;
            }
        }

        /// <summary>
        /// Gets the number of children that is owned by this node.
        /// </summary>
        public int ChildCount
        {
            get
            {
                return m_children.Count;
            }
        }

        /// <summary>
        /// Gets if the node contains children.
        /// </summary>
        public bool HasChildren
        {
            get
            {
                return m_children.Count > 0;
            }
        }

        /// <summary>
        /// Gets the node's children.
        /// </summary>
        public NodeCollection Children
        {
            get
            {
                return m_children;
            }
            set
            {
                m_children = value;
            }
        }

        /// <summary>
        /// Gets the number of meshes referenced by this node.
        /// </summary>
        public int MeshCount
        {
            get
            {
                return m_meshes.Count;
            }
        }

        /// <summary>
        /// Gets if the node contains mesh references.
        /// </summary>
        public bool HasMeshes
        {
            get
            {
                return m_meshes.Count > 0;
            }
        }

        /// <summary>
        /// Gets the indices of the meshes referenced by this node. Meshes can be
        /// shared between nodes, so there is a mesh collection owned by the scene
        /// that each node can reference.
        /// </summary>
        public List<int> MeshIndices
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
        /// Gets the node's metadata container.
        /// </summary>
        public Metadata Metadata
        {
            get
            {
                return m_metaData;
            }
        }

        /// <summary>
        /// Constructs a new instance of the <see cref="Node"/> class.
        /// </summary>
        public Node()
        {
            m_name = String.Empty;
            m_transform = Matrix4x4.Identity;
            m_parent = null;
            m_children = new NodeCollection(this);
            m_meshes = new List<int>();
            m_metaData = new Metadata();
        }

        /// <summary>
        /// Constructs a new instance of the <see cref="Node"/> class.
        /// </summary>
        /// <param name="name">Name of the node</param>
        public Node(String name)
            : this()
        {
            m_name = name;
        }

        /// <summary>
        /// Constructs a new instance of the <see cref="Node"/> class.
        /// </summary>
        /// <param name="name">Name of the node</param>
        /// <param name="parent">Parent of the node</param>
        public Node(String name, Node parent)
            : this()
        {
            m_name = name;
            m_parent = parent;
        }

        //Internal use - sets the node parent in NodeCollection
        internal void SetParent(Node parent)
        {
            m_parent = parent;
        }

        /// <summary>
        /// Finds a node with the specific name, which may be this node
        /// or any children or children's children, and so on, if it exists.
        /// </summary>
        /// <param name="name">Node name</param>
        /// <returns>The node or null if it does not exist</returns>
        public Node FindNode(String name)
        {
            if (name.Equals(m_name))
                return this;

            if (HasChildren)
            {
                foreach (Node child in m_children)
                {
                    Node found = child.FindNode(name);
                    if (found != null)
                        return found;
                }
            }
            //No child found
            return null;
        }

        private IntPtr ToNativeRecursive(IntPtr parentPtr, Node node)
        {
            if (node == null)
                return IntPtr.Zero;

            int sizeofNative = MemoryHelper.SizeOf<AiNode>();

            //Allocate the memory that will hold the node
            IntPtr nodePtr = MemoryHelper.AllocateMemory(sizeofNative);

            //First fill the native struct
            AiNode nativeValue;
            nativeValue.Name = new AiString(node.m_name);
            nativeValue.Transformation = node.m_transform;
            nativeValue.Parent = parentPtr;

            nativeValue.NumMeshes = (uint)node.m_meshes.Count;
            nativeValue.Meshes = MemoryHelper.ToNativeArray<int>(node.m_meshes.ToArray());
            nativeValue.MetaData = IntPtr.Zero;

            //If has metadata, create it, otherwise it should be NULL
            if (m_metaData.Count > 0)
                nativeValue.MetaData = MemoryHelper.ToNativePointer<Metadata, AiMetadata>(m_metaData);

            //Now descend through the children
            nativeValue.NumChildren = (uint)node.m_children.Count;

            int numChildren = (int)nativeValue.NumChildren;
            int stride = IntPtr.Size;

            IntPtr childrenPtr = IntPtr.Zero;

            if (numChildren > 0)
            {
                childrenPtr = MemoryHelper.AllocateMemory(numChildren * IntPtr.Size);

                for (int i = 0; i < numChildren; i++)
                {
                    IntPtr currPos = MemoryHelper.AddIntPtr(childrenPtr, stride * i);
                    Node child = node.m_children[i];

                    IntPtr childPtr = IntPtr.Zero;

                    //Recursively create the children and its children
                    if (child != null)
                    {
                        childPtr = ToNativeRecursive(nodePtr, child);
                    }

                    //Write the child's node ptr to our array
                    MemoryHelper.Write<IntPtr>(currPos, childPtr);
                }
            }

            //Finall finish writing to the native struct, and write the whole thing to the memory we allocated previously
            nativeValue.Children = childrenPtr;
            MemoryHelper.Write<AiNode>(nodePtr, nativeValue);

            return nodePtr;
        }

        #region IMarshalable Implemention

        /// <summary>
        /// Gets a value indicating whether this instance is native blittable.
        /// </summary>
        bool IMarshalable<Node, AiNode>.IsNativeBlittable
        {
            get { return true; }
        }

        /// <summary>
        /// Writes the managed data to the native value.
        /// </summary>
        /// <param name="thisPtr">Optional pointer to the memory that will hold the native value.</param>
        /// <param name="nativeValue">Output native value</param>
        void IMarshalable<Node, AiNode>.ToNative(IntPtr thisPtr, out AiNode nativeValue)
        {
            nativeValue.Name = new AiString(m_name);
            nativeValue.Transformation = m_transform;
            nativeValue.Parent = IntPtr.Zero;

            nativeValue.NumMeshes = (uint)m_meshes.Count;
            nativeValue.Meshes = IntPtr.Zero;
            nativeValue.MetaData = IntPtr.Zero;

            //If has metadata, create it, otherwise it should be NULL
            if (m_metaData.Count > 0)
                nativeValue.MetaData = MemoryHelper.ToNativePointer<Metadata, AiMetadata>(m_metaData);

            if (nativeValue.NumMeshes > 0)
                nativeValue.Meshes = MemoryHelper.ToNativeArray<int>(m_meshes.ToArray());

            //Now descend through the children
            nativeValue.NumChildren = (uint)m_children.Count;

            int numChildren = (int)nativeValue.NumChildren;
            int stride = IntPtr.Size;

            IntPtr childrenPtr = IntPtr.Zero;

            if (numChildren > 0)
            {
                childrenPtr = MemoryHelper.AllocateMemory(numChildren * IntPtr.Size);

                for (int i = 0; i < numChildren; i++)
                {
                    IntPtr currPos = MemoryHelper.AddIntPtr(childrenPtr, stride * i);
                    Node child = m_children[i];

                    IntPtr childPtr = IntPtr.Zero;

                    //Recursively create the children and its children
                    if (child != null)
                    {
                        childPtr = ToNativeRecursive(thisPtr, child);
                    }

                    //Write the child's node ptr to our array
                    MemoryHelper.Write<IntPtr>(currPos, childPtr);
                }
            }

            //Finally finish writing to the native struct
            nativeValue.Children = childrenPtr;
        }

        /// <summary>
        /// Frees unmanaged memory created by <see cref="IMarshalable{Node, AiNode}.ToNative"/>.
        /// </summary>
        /// <param name="nativeValue">Native value to free</param>
        /// <param name="freeNative">True if the unmanaged memory should be freed, false otherwise.</param>
        public static void FreeNative(IntPtr nativeValue, bool freeNative)
        {
            if (nativeValue == IntPtr.Zero)
                return;

            AiNode aiNode = MemoryHelper.Read<AiNode>(nativeValue);

            if (aiNode.NumMeshes > 0 && aiNode.Meshes != IntPtr.Zero)
                MemoryHelper.FreeMemory(aiNode.Meshes);

            if (aiNode.NumChildren > 0 && aiNode.Children != IntPtr.Zero)
                MemoryHelper.FreeNativeArray<AiNode>(aiNode.Children, (int)aiNode.NumChildren, FreeNative, true);

            if (aiNode.MetaData != IntPtr.Zero)
                Metadata.FreeNative(aiNode.MetaData, true);

            if (freeNative)
                MemoryHelper.FreeMemory(nativeValue);
        }

        public void FromNative(in AiNode nativeValue)
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}
