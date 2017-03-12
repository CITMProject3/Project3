using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScripts
{
    public class Test
    {
        public int age;
        private float height;
        private String name;

        public Test()
        {
            age = 29;
            height = 1.71f;
            name = "Carlos";
        }

        ~Test()
        { }

        public void Process()
        {
            throw new NotImplementedException("Not implemented yet");
        }

        public String GetName()
        {
            return name;
        }

        public void DoNothing()
        { }
    }
}
