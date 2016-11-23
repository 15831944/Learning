using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using System.IO;

namespace L01
{
    struct TestCase
    {
        public string InputData;
        public string OutputData;
    }

    struct Movie
    {
        public string Name;
        public DateTime ReleaseDate;
        public string[] Genres;
    }
    class Program
    {
        static void Main(string[] args)
        {
            var content = File.ReadAllText(@"case.json");
            var cases = JsonConvert.DeserializeObject<TestCase[]>(content);

            string json = File.ReadAllText(@"data.json");

            Movie[] m = JsonConvert.DeserializeObject<Movie[]>(json);

            string name = m[0].Name;

            return;
        }
    }
}
