using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace L01_HelloMongoDB
{
    public class Client
    {
        public string FirstName { get; set; }
        public int Age { get; set; }
        public List<string> Address { get; set; } = new List<string>();
        public Contact Contact { get; set; } = new Contact();
    }
}
