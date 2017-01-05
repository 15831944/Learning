using MongoDB.Bson;
using MongoDB.Bson.Serialization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace L02_Bson
{
    public class Program
    {
        public static void Main(string[] args)
        {
            // WhatIsBson();
            TestBsonDocument();
        }

        static void WhatIsBson()
        {
            var doc = BsonDocument.Parse("{\"name\":\"ShangHai\",\"age\":25}");
            Console.WriteLine($"Json文档:{doc.ToJson()}");
            // 转换成为二进制格式
            var bson = doc.ToBson();
            Console.WriteLine($"转换为Bson之后:{BitConverter.ToString(bson)}");
            // 从Bson数据中反序列化对象
            var deserializedDoc = BsonSerializer.Deserialize<BsonDocument>(bson);
            Console.WriteLine($"从Bson数据中反序列化, 得到BsonDocument对象:{deserializedDoc}");
        }
        static void TestBsonDocument()
        {
            // BsonDocument支持以多重嵌套方式创建文档
            var document = new BsonDocument()
            {
                { "name", "MongoDB" },
                { "type", "Database" },
                { "count", 1 },
                { "info", new BsonDocument()
                    {
                        { "x", 203 },
                        { "y", 102 }
                    }
                }
            };
            Console.WriteLine($"ToString()方法输出Json字符串\n{document.ToString()}");

            // 文档可以看成是key/value的有序集合
            Console.WriteLine("访问文档中指定名字的属性的值");
            var firstElementValue = document[0];       // 通过索引访问
            var countElementValue = document["count"]; // 通过名字访问
            var infoElementValue = document[document.ElementCount - 1]; // 最后一个元素, 引用子文档

            Console.WriteLine($"{firstElementValue},{countElementValue.AsInt32 + 100}");
            Console.WriteLine($"{firstElementValue.IsString}"); // true
            Console.WriteLine($"{infoElementValue.ToJson()}的类型:{infoElementValue.BsonType}");

            Console.WriteLine("查询某指定名字的元素是否存在于文档中");
            Console.WriteLine(document.Contains("info")); // true
            
            // 自己及所包含的所有子文档, 均被转换为dictionary
            Console.WriteLine("转换为Dictionary");
            var dict = document.ToDictionary();
            // 显示dictionary中的内容
            foreach (var key in dict.Keys)
            {
                Console.WriteLine($"{key}:{dict[key]}");
            }
            Console.WriteLine();

            Console.WriteLine("解析Json字符串为BsonDocument对象");
            var newDoc = BsonDocument.Parse("{a:1,b:2,c:3}");
            newDoc.Add("d", 4);
            Console.WriteLine(newDoc.ToString());
            newDoc.Set("a", "Hello"); // 修改a元素
            newDoc.RemoveAt(2); // 移除第3个元素
            Console.WriteLine(newDoc);

            Console.WriteLine("文档克隆");
            var cloneDoc = newDoc.Clone();
            Console.WriteLine(cloneDoc);

            Console.WriteLine("文档合并");
            var doc1 = BsonDocument.Parse("{x:1}");
            var doc2 = BsonDocument.Parse("{y:2}");
            var mergedDoc = doc1.Merge(doc2);
            Console.WriteLine(mergedDoc); // { "x" : 1, "y" : 2 }

            Console.WriteLine("文档判等");
            string json = "{\"x\":1,\"y\":2}";
            var doc3 = BsonDocument.Parse(json);
            Console.WriteLine(mergedDoc.CompareTo(doc3)); // 0
            Console.WriteLine(mergedDoc == doc3); // true
            Console.WriteLine(mergedDoc.Equals(doc3)); // true
        }
    }
}
