using System;

namespace ConsoleApplication
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
