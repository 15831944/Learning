using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;

// For more information on enabling MVC for empty projects, visit http://go.microsoft.com/fwlink/?LinkID=397860

namespace L01_Hello_Asp_Net_Core.Controllers
{
    public class HomeController : Controller
    {
        // GET: /<controller>/
        public IActionResult Index(string str)
        {
            ViewBag.Code = @"int main()\n{\n\treturn 0;\n}\n";
            return View();
        }
    }
}
