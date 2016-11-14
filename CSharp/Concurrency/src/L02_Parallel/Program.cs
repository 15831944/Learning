using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace L02_Parallel
{
    public class Program
    {
        static int cal()
        {
            int[] values = new int[49];
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = i+1;
            }
            object mutex = new object();
            int result = 0;
            Parallel.ForEach(
                source: values,
                localInit: () => 0,
                body: (item, state, localValue) => item++,
                localFinally: localValue => {
                    lock (mutex)
                        result += localValue;
                });
            return result;
        }
        public static void Main(string[] args)
        {
            int a = cal();
            Console.WriteLine(a);
            //foreach (var item in a)
            //{
            //    Console.WriteLine(item);
            //}
            while (true) ;
        }
    }
}
