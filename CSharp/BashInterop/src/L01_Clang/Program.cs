using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;

namespace L01_Clang
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Process process = new Process();
            process.StartInfo.FileName = "clang-3.5";
            process.StartInfo.Arguments = "-v";
            process.Start();
        }
    }
}
