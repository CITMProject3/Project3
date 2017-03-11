using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScripts
{
    public class Test
    {
        public int age = 42;
        private float height = 1.70f;
        private String name;

        public Test(String name)
        { }

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
