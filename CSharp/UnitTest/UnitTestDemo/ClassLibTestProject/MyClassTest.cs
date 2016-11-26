using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using ClassLib;

namespace ClassLibTestProject
{
    [TestClass]
    public class MyClassTest
    {
        [TestMethod]
        public void TestDoubleValue()
        {
            MyClass target = new MyClass();

            int value = 1;
            int expected = 2;

            int actual = target.DoubleValue(value);

            Assert.AreEqual(expected, actual);
        }
    }
}
