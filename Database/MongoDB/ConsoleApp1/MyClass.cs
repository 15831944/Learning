using MongoDB.Bson.Serialization.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace CsharpCRUD
{
    [BsonDiscriminator("myclass")]
    public class MyClass
    {
        [BsonId]
        public int MyClassId { get; set; }
        public string Info { get; set; }
    }
}
