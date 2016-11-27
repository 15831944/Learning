using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using MailKit.Net.Smtp;
using MimeKit;
using MailKit.Security;

namespace L01
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var mail = new MimeMessage();
            mail.From.Add(new MailboxAddress(@"stark.shang", @"344347752@qq.com"));
            mail.To.Add(new MailboxAddress("", @"531779485@qq.com"));
            //mail.Cc.Add(new MailboxAddress("", @"429701238@qq.com"));
            mail.Cc.Add(new MailboxAddress("", @"344347752@qq.com"));
            mail.Subject = @"Test";
            mail.Body = new TextPart(MimeKit.Text.TextFormat.Html)
            {
                Text = "<H2>This is a test mail message.</H2>"
            };

            using (var client = new SmtpClient())
            {
                client.Connect("smtp.qq.com", 465, true);
                client.Authenticate("344347752@qq.com", "srbqqdtjmeaebhje");
                client.Send(mail);
                client.Disconnect(true);
            }
        }
    }
}
