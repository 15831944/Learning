using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using L02_Hello_Web_Api.Models;
// For more information on enabling Web API for empty projects, visit http://go.microsoft.com/fwlink/?LinkID=397860

namespace L02_Hello_Web_Api.Controllers
{
    [Route("api/[controller]")]
    public class MyServiceController : Controller
    {
        // GET: api/values
        [HttpGet]
        public IEnumerable<string> Get()
        {
            return new string[] { "value1", "value2" };
        }

        // GET api/values/5
        [HttpGet("{id}")]
        public IActionResult Get(int id)
        {
            return Ok(new MyClass() { Id = id, Info = "Hello" });
        }

        // POST api/values
        [HttpPost]
        public IActionResult Post([FromBody]MyClass obj)
        {
            obj.Id = new Random().Next(1, 1000);
            return Created(obj.Id.ToString(), obj);
        }

        // PUT api/values/5
        [HttpPut("{id}")]
        public void Put(int id, [FromBody]string value)
        {
        }

        // DELETE api/values/5
        [HttpDelete("{id}")]
        public void Delete(int id)
        {
        }
    }
}
