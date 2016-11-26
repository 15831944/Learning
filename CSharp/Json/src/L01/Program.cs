using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Newtonsoft.Json;
using System.IO;

namespace L01
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var dir = new DirectoryInfo(".");
            Console.WriteLine(ConfigInfo.CellRoot);
        }
    }
}
