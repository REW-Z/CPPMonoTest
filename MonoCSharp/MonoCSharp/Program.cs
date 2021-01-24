using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.CompilerServices;

namespace MonoCSharp
{
    public class Scene
    {
        private IntPtr handle_scene = (IntPtr)0;

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Scene GetMainScene();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static GameObject[] GetGameObjects();
        
    }



    public class GameObject
    {
        private IntPtr handle_gameobj = (IntPtr)0;
        public Component[] components
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }
        

        public int ComponentCount
        {
            get
            {
                return get_ComponentCount(this.handle_gameobj);
            }
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static int get_ComponentCount(IntPtr handle);
    }




    public class Component
    {
        private IntPtr handle_com = (IntPtr)0;

        public virtual int ID
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }
    }

    public class Transform: Component
    {
        private IntPtr handle_transform = (IntPtr)0;

        public override int ID
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
        }
    }


    public class TestClass
    {
        public static void HelloWorld()
        {
            Console.WriteLine("Hello CSharp!");
            Console.ReadLine();
        }
        public static void Test()
        {
            GameObject[] objs = Scene.GetGameObjects();
            Console.WriteLine("对象数： " + objs.Length.ToString());
            Console.WriteLine("对象[0]组件数： " + objs[0].ComponentCount.ToString());
            Component com0 = (objs[0].components[0]);
            Console.WriteLine("对象[0]第[0]个组件ID： " + com0.ID.ToString());
            Component com1 = (objs[0].components[1]);
            Console.WriteLine("对象[0]第[1]个组件ID： " + com1.ID.ToString());
            Console.ReadLine();
        }
    }
    
}
