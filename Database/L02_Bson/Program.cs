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
            Console.WriteLine($"Json�ĵ�:{doc.ToJson()}");
            // ת����Ϊ�����Ƹ�ʽ
            var bson = doc.ToBson();
            Console.WriteLine($"ת��ΪBson֮��:{BitConverter.ToString(bson)}");
            // ��Bson�����з����л�����
            var deserializedDoc = BsonSerializer.Deserialize<BsonDocument>(bson);
            Console.WriteLine($"��Bson�����з����л�, �õ�BsonDocument����:{deserializedDoc}");
        }
        static void TestBsonDocument()
        {
            // BsonDocument֧���Զ���Ƕ�׷�ʽ�����ĵ�
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
            Console.WriteLine($"ToString()�������Json�ַ���\n{document.ToString()}");

            // �ĵ����Կ�����key/value�����򼯺�
            Console.WriteLine("�����ĵ���ָ�����ֵ����Ե�ֵ");
            var firstElementValue = document[0];       // ͨ����������
            var countElementValue = document["count"]; // ͨ�����ַ���
            var infoElementValue = document[document.ElementCount - 1]; // ���һ��Ԫ��, �������ĵ�

            Console.WriteLine($"{firstElementValue},{countElementValue.AsInt32 + 100}");
            Console.WriteLine($"{firstElementValue.IsString}"); // true
            Console.WriteLine($"{infoElementValue.ToJson()}������:{infoElementValue.BsonType}");

            Console.WriteLine("��ѯĳָ�����ֵ�Ԫ���Ƿ�������ĵ���");
            Console.WriteLine(document.Contains("info")); // true
            
            // �Լ������������������ĵ�, ����ת��Ϊdictionary
            Console.WriteLine("ת��ΪDictionary");
            var dict = document.ToDictionary();
            // ��ʾdictionary�е�����
            foreach (var key in dict.Keys)
            {
                Console.WriteLine($"{key}:{dict[key]}");
            }
            Console.WriteLine();

            Console.WriteLine("����Json�ַ���ΪBsonDocument����");
            var newDoc = BsonDocument.Parse("{a:1,b:2,c:3}");
            newDoc.Add("d", 4);
            Console.WriteLine(newDoc.ToString());
            newDoc.Set("a", "Hello"); // �޸�aԪ��
            newDoc.RemoveAt(2); // �Ƴ���3��Ԫ��
            Console.WriteLine(newDoc);

            Console.WriteLine("�ĵ���¡");
            var cloneDoc = newDoc.Clone();
            Console.WriteLine(cloneDoc);

            Console.WriteLine("�ĵ��ϲ�");
            var doc1 = BsonDocument.Parse("{x:1}");
            var doc2 = BsonDocument.Parse("{y:2}");
            var mergedDoc = doc1.Merge(doc2);
            Console.WriteLine(mergedDoc); // { "x" : 1, "y" : 2 }

            Console.WriteLine("�ĵ��е�");
            string json = "{\"x\":1,\"y\":2}";
            var doc3 = BsonDocument.Parse(json);
            Console.WriteLine(mergedDoc.CompareTo(doc3)); // 0
            Console.WriteLine(mergedDoc == doc3); // true
            Console.WriteLine(mergedDoc.Equals(doc3)); // true
        }
    }
}
