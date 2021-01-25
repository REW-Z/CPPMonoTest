using System;
using System.Reflection;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.CompilerServices;

namespace MonoCSharp
{
    public struct Vec3
    {
        public float x, y, z;
        public Vec3(float newx, float newy, float newz)
        {
            x = newx;
            y = newy;
            z = newz;
        }
    }

    public class Scene
    {
        private IntPtr handle = (IntPtr)0;

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Scene GetMainScene();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static GameObject[] GetGameObjects();
        
    }

    public class Object
    {
        protected IntPtr handle = (IntPtr)0;

        public IntPtr Handle
        {
            get { return this.handle; }
        }

        public static void Destory(Object obj)
        {
            DestroyViaPtr(obj.handle);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DestroyViaPtr(IntPtr ptr);
    }
    
    public class GameObject: Object
    {
        
        public Component[] components
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        public int ComponentCount
        {
            get
            {
                return get_ComponentCount(this.handle);
            }
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static int get_ComponentCount(IntPtr handle);

        public void AddComponent(Component component)
        {
            AddComponentViaPtr(this.Handle, component.Handle);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static int AddComponentViaPtr(IntPtr handleGameObject, IntPtr handleComponent);
    }
    
    public class Component: Object
    {
        public Component()
        {
            this.handle = ComponentConstruct();
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern IntPtr ComponentConstruct();


        public virtual int TID
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

    }

    public class Transform: Component
    {
        public override int TID
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }

        public Vec3 Position
        {
            get { return get_Position(this.Handle); }
            set { set_Position(this.Handle, value); }
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern Vec3 get_Position(IntPtr ptr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void set_Position(IntPtr ptr, Vec3 pos);

        public Vec3 Rotation
        {
            get { return get_Rotation(this.Handle); }
            set { set_Rotation(this.Handle, value); }
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern Vec3 get_Rotation(IntPtr ptr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void set_Rotation(IntPtr ptr, Vec3 rot);

        public Vec3 Scale
        {
            get { return get_Scale(this.Handle); }
            set { set_Scale(this.Handle, value); }
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern Vec3 get_Scale(IntPtr ptr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void set_Scale(IntPtr ptr, Vec3 scale);
    }

    public class TestClass
    {
        public static void HelloWorld()
        {
            Console.WriteLine("Hello CSharp!");
            Console.ReadLine();
        }
        public static void LogScene()
        {
            GameObject[] gameObjs = Scene.GetGameObjects();

            foreach (GameObject gameobject in gameObjs)
            {
                Console.WriteLine("==============GameObject===============");

                Component[] components = gameobject.components;

                foreach (Component com in components)
                {
                    Console.WriteLine("----------------");
                    Console.WriteLine("TID：" + com.TID);

                    Type type = com.GetType();
                    if(type == typeof(Transform))
                    {
                        Transform t = (Transform)com;
                        Console.WriteLine("Value: (" + t.Position.x + "," + t.Position.y + "," + t.Position.z + ")");
                        Console.WriteLine("Value: (" + t.Rotation.x + "," + t.Rotation.y + "," + t.Rotation.z + ")");
                        Console.WriteLine("Value: (" + t.Scale.x + "," + t.Scale.y + "," + t.Scale.z + ")");
                    }
                    else
                    {
                        Console.WriteLine("Value: (NONE)");
                    }
                    Console.WriteLine("----------------");
                }

                Console.WriteLine("=======================================");
            }
        }
        public static void Start()
        {
            LogScene();

            Console.WriteLine("\n\n\n删除组件...\n\n\n");
            Component com = Scene.GetGameObjects()[0].components[0];//删除Transform
            Object.Destory(com);

            LogScene();

            Console.WriteLine("\n\n\n添加组件...\n\n\n");
            Transform newcom = new Transform();
            newcom.Position = new Vec3(9f, 9f, 9f);
            newcom.Rotation = new Vec3(90f, 180f, 270f);
            newcom.Scale = new Vec3(1.1f, 1.1f, 1.1f);
            Scene.GetGameObjects()[0].AddComponent(newcom);//添加新的Transform

            LogScene();



            Console.ReadLine();
        }
        public static void Update()
        {
            Console.WriteLine("Update...");
        }
    }
    
}
