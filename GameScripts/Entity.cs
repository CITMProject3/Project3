using System;
using System.Runtime.InteropServices;

namespace GameScripts
{
    public class Entity
    {
        [DllImport("EngineFunctions.dll")]
        public static extern void DisplayHelloFromDLL();

        public int age;
        private float height;
        private String name;

        public Entity()
        {
            DisplayHelloFromDLL();

            age = 29;
            height = 1.71f;
            name = "Carlos";
        }

        ~Entity()
        { }

        public void Process()
        {
            throw new NotImplementedException("Not implemented yet");
        }

        //public String GetName()
        //{
        //    return name;
        //}

        public void DoNothing()
        { }
    }
}
