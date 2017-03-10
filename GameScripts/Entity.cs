﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScripts
{
    public class Entity
    {
        private String name;

        public Entity(String name)
        {
            this.name = name;
            System.Console.WriteLine("Entity " + name + " constructed");
        }

        ~Entity()
        {
            System.Console.WriteLine("Entity " + name + " destructed");
        }

        public void Process()
        {
            throw new NotImplementedException("Not implemented yet");
        }

        public String GetName()
        {
            return name;
        }
    }
}
