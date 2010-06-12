using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Windows.Forms;

namespace h264_Sucelje
{
    static class Program
    {
        static public Form1 forma;
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            //Thread t = new Thread(new ThreadStart(Radi));
            //t.Start();
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            forma = new Form1();
            Application.Run(forma);
        }

        static public void Radi()
        {
            int brojac = 0;
            while (true)
            {
                Thread.Sleep(1000);
                brojac++;
                forma.Poruka = (brojac / 10).ToString() + "." + (brojac % 10).ToString() + " sec.";
                MessageBox.Show("Jupi");
            }
        }
    }
}
