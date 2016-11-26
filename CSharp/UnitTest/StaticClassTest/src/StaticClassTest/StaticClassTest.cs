using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace StaticClassTest
{
    [TestClass]
    public class StaticClassTest
    {
        [TestMethod]
        public void MemberTest()
        {
            Assert.AreEqual(StaticClass.StaticClass.Member, "Hello World");
        }
    }
}
