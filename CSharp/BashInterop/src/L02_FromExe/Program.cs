using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace L02_FromExe
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var path = @"C:\Workspace\Coding\Learning\CSharp\BashInterop\src\L02_FromExe";
            Process process = new Process();
            process.StartInfo.FileName = Path.Combine(path, "main.exe");
            process.StartInfo.Arguments = "3";
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.RedirectStandardInput = true;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.CreateNoWindow = true;
            process.OutputDataReceived += (object sender, DataReceivedEventArgs e) => {
                if (e.Data == null) return;
                var match = Regex.Match(e.Data, @"\[[i,o,e]\]");
                if (match.Success) {
                    switch (match.Value) {
                        case "[i]":
                            var param = "-i 1 0 0 0 -o 0 0 0 0 0 0";
                            process.StandardInput.WriteLine(param);
                            break;
                        case "[o]":
                            Console.WriteLine(e.Data);
                            break;
                        case "[e]":
                            Console.WriteLine(e.Data);
                            break;
                        default: break;
                    }
                }
            };
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            process.WaitForExit();
        }
    }
}
