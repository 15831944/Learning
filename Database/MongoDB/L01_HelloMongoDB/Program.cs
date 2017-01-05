using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using MongoDB;
using MongoDB.Driver;
using MongoDB.Bson;
using MongoDB.Bson.IO;

namespace L01_HelloMongoDB
{
    public class Program
    {
        public static void Main(string[] args)
        {
            // ReadJson();
            TestPOCOJsonBson();
        }

        static void ReadJson()
        {
            var jsonString = "{a:1}";
            using (var reader = new JsonReader(jsonString))
            {
                // 读入开始标记
                reader.ReadStartDocument();
                // 读入元素名字
                string fieldName = reader.ReadName();
                Console.WriteLine(fieldName);
                // 读取元素值
                int value = reader.ReadInt32();
                Console.WriteLine(value);
                // 读入结束标记
                reader.ReadEndDocument();
            }
        }

        static void TestPOCOJsonBson()
        {
            var client = new Client
            {
                Age = 25,
                FirstName = "Hai"
            };
            client.Address.Add("Shanghai");
            client.Address.Add("Zhejiang");
            client.Contact.Email = "stark.shang@outlook.com";
            client.Contact.Phone = "18217286025";

            // MongoDB提供了将POCO对象直接转换为Json字符串的功能
            var json = client.ToJson();
            Console.WriteLine(json);
            // 可将POCO对象直接转换为Bson, 得到一个字节数组
            var bson = client.ToBson();
            Console.WriteLine(bson);
            // 可将POCO对象直接转换为BsonDocument, 可以直接操作它
            var bsonDoc = client.ToBsonDocument();
            Console.WriteLine(bsonDoc);
        }
    }
}
