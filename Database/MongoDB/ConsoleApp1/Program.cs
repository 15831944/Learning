using MongoDB.Bson;
using MongoDB.Driver;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace CsharpCRUD
{
    public class Program
    {
        public static void Main(string[] args)
        {
            // CreateDatabaseAndCollection().Wait();
            // InsertData().Wait();
            InsertPOCO().Wait();
        }
        static async Task CreateDatabaseAndCollection()
        {
            var client = new MongoClient();
            using (var cursor = await client.ListDatabasesAsync())
            {
                await cursor.ForEachAsync(d => Console.WriteLine(d.ToString()));
            }

            var db = client.GetDatabase("test");
            var collection = db.GetCollection<BsonDocument>("foo");
        }

        static async Task InsertData()
        {
            var _client = new MongoClient();
            var _database = _client.GetDatabase("test");
            var collection = _database.GetCollection<BsonDocument>("myCollection");
            BsonDocument document = BsonDocument.Parse("{x:1,y:2}");
            // 插入单条记录
            // Note : InsertManyAsync()可以插入多条记录
            await collection.InsertOneAsync(document);
            Console.WriteLine("插入结束");
            // 显示处理结果
            var cursor = await _database.ListCollectionsAsync();
            await cursor.ForEachAsync(d => Console.WriteLine(d.ToString()));
            var docs = await collection.Find(new BsonDocument()).ToListAsync();
            docs.ForEach(d => Console.WriteLine(d.ToString()));
        }

        static async Task InsertPOCO()
        {
            var _client = new MongoClient();
            var _database = _client.GetDatabase("test");
            // 指明文档中保存的对象模型为MyClass
            var collection = _database.GetCollection<MyClass>("myCollection");
            int ranValue = new Random().Next(1, 10000);
            // 注意MyClassId字段在定义时加不加[BsonId], 有重要影响
            // 如果不加, 则MongoDB会自己添加一个objectId作为对象标识
            // 如果加了, 则要求值必须唯一, 否则, 在插入时会抛出异常报告"Key重复"
            MyClass obj = new MyClass()
            {
                MyClassId = ranValue,
                Info = $"自定义的对象{ranValue}"
            };

            await collection.InsertOneAsync(obj);
            Console.WriteLine("POCO对象插入结束");
            var cols = _database.GetCollection<BsonDocument>("myCollection");
            var docs = await collection.Find(new BsonDocument()).ToListAsync();
            docs.ForEach(d => Console.WriteLine(d.ToString()));
        }
    }
}
