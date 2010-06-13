namespace h264_Sucelje
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.FrameStart = new System.Windows.Forms.NumericUpDown();
            this.FrameEnd = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this._textStatus = new System.Windows.Forms.Label();
            this.Kodiraj = new System.Windows.Forms.Button();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.label4 = new System.Windows.Forms.Label();
            this._qp = new System.Windows.Forms.NumericUpDown();
            this._podaci = new System.Windows.Forms.DataGridView();
            this.button1 = new System.Windows.Forms.Button();
            this._velicinaProzora = new System.Windows.Forms.NumericUpDown();
            this.label5 = new System.Windows.Forms.Label();
            this._toleriranaGreska = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this._osnovnoInter = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label7 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.button2 = new System.Windows.Forms.Button();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown2 = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.Dekodiraj = new System.Windows.Forms.Button();
            this.button4 = new System.Windows.Forms.Button();
            this.button5 = new System.Windows.Forms.Button();
            this.label10 = new System.Windows.Forms.Label();
            this.statusDekoder = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.FrameStart)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.FrameEnd)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._qp)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._podaci)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._velicinaProzora)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._toleriranaGreska)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).BeginInit();
            this.SuspendLayout();
            // 
            // FrameStart
            // 
            this.FrameStart.Location = new System.Drawing.Point(132, 150);
            this.FrameStart.Maximum = new decimal(new int[] {
            5000,
            0,
            0,
            0});
            this.FrameStart.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.FrameStart.Name = "FrameStart";
            this.FrameStart.Size = new System.Drawing.Size(52, 20);
            this.FrameStart.TabIndex = 7;
            this.FrameStart.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.FrameStart.KeyUp += new System.Windows.Forms.KeyEventHandler(this.FrameStart_KeyUp);
            // 
            // FrameEnd
            // 
            this.FrameEnd.Location = new System.Drawing.Point(132, 186);
            this.FrameEnd.Maximum = new decimal(new int[] {
            5000,
            0,
            0,
            0});
            this.FrameEnd.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.FrameEnd.Name = "FrameEnd";
            this.FrameEnd.Size = new System.Drawing.Size(52, 20);
            this.FrameEnd.TabIndex = 9;
            this.FrameEnd.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(20, 152);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(68, 13);
            this.label1.TabIndex = 14;
            this.label1.Text = "Početni frejm";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(21, 188);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(67, 13);
            this.label2.TabIndex = 8;
            this.label2.Text = "Završni frejm";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(20, 276);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(40, 13);
            this.label3.TabIndex = 12;
            this.label3.Text = "Status:";
            // 
            // _textStatus
            // 
            this._textStatus.AutoSize = true;
            this._textStatus.Location = new System.Drawing.Point(67, 276);
            this._textStatus.Name = "_textStatus";
            this._textStatus.Size = new System.Drawing.Size(46, 13);
            this._textStatus.TabIndex = 13;
            this._textStatus.Text = "Čekanje";
            // 
            // Kodiraj
            // 
            this.Kodiraj.Location = new System.Drawing.Point(6, 245);
            this.Kodiraj.Name = "Kodiraj";
            this.Kodiraj.Size = new System.Drawing.Size(192, 26);
            this.Kodiraj.TabIndex = 11;
            this.Kodiraj.Text = "Pokreni koder";
            this.Kodiraj.UseVisualStyleBackColor = true;
            this.Kodiraj.Click += new System.EventHandler(this.Kodiraj_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(21, 121);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(22, 13);
            this.label4.TabIndex = 5;
            this.label4.Text = "QP";
            // 
            // _qp
            // 
            this._qp.Location = new System.Drawing.Point(132, 119);
            this._qp.Maximum = new decimal(new int[] {
            30,
            0,
            0,
            0});
            this._qp.Minimum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this._qp.Name = "_qp";
            this._qp.Size = new System.Drawing.Size(52, 20);
            this._qp.TabIndex = 6;
            this._qp.Value = new decimal(new int[] {
            12,
            0,
            0,
            0});
            // 
            // _podaci
            // 
            this._podaci.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this._podaci.Location = new System.Drawing.Point(241, 43);
            this._podaci.Name = "_podaci";
            this._podaci.Size = new System.Drawing.Size(654, 355);
            this._podaci.TabIndex = 3;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(6, 213);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(95, 26);
            this.button1.TabIndex = 10;
            this.button1.Text = "Učitaj Y4M";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // _velicinaProzora
            // 
            this._velicinaProzora.Location = new System.Drawing.Point(132, 89);
            this._velicinaProzora.Maximum = new decimal(new int[] {
            256,
            0,
            0,
            0});
            this._velicinaProzora.Minimum = new decimal(new int[] {
            8,
            0,
            0,
            0});
            this._velicinaProzora.Name = "_velicinaProzora";
            this._velicinaProzora.Size = new System.Drawing.Size(52, 20);
            this._velicinaProzora.TabIndex = 4;
            this._velicinaProzora.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(21, 91);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(82, 13);
            this.label5.TabIndex = 3;
            this.label5.Text = "Veličina prozora";
            // 
            // _toleriranaGreska
            // 
            this._toleriranaGreska.Location = new System.Drawing.Point(132, 60);
            this._toleriranaGreska.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this._toleriranaGreska.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            this._toleriranaGreska.Name = "_toleriranaGreska";
            this._toleriranaGreska.Size = new System.Drawing.Size(52, 20);
            this._toleriranaGreska.TabIndex = 2;
            this._toleriranaGreska.Value = new decimal(new int[] {
            3,
            0,
            0,
            0});
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(21, 62);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(89, 13);
            this.label6.TabIndex = 1;
            this.label6.Text = "Tolerirana greška";
            // 
            // _osnovnoInter
            // 
            this._osnovnoInter.AutoSize = true;
            this._osnovnoInter.Location = new System.Drawing.Point(24, 28);
            this._osnovnoInter.Name = "_osnovnoInter";
            this._osnovnoInter.Size = new System.Drawing.Size(152, 17);
            this._osnovnoInter.TabIndex = 0;
            this._osnovnoInter.Text = "Osnovno Inter predviđanje";
            this._osnovnoInter.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.button4);
            this.groupBox1.Controls.Add(this._osnovnoInter);
            this.groupBox1.Controls.Add(this.FrameStart);
            this.groupBox1.Controls.Add(this._toleriranaGreska);
            this.groupBox1.Controls.Add(this.FrameEnd);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this._velicinaProzora);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.button1);
            this.groupBox1.Controls.Add(this._textStatus);
            this.groupBox1.Controls.Add(this.Kodiraj);
            this.groupBox1.Controls.Add(this._qp);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Location = new System.Drawing.Point(22, 171);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(203, 293);
            this.groupBox1.TabIndex = 2;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Koder";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(249, 18);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(89, 13);
            this.label7.TabIndex = 17;
            this.label7.Text = "Statistika kodera:";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.label10);
            this.groupBox2.Controls.Add(this.statusDekoder);
            this.groupBox2.Controls.Add(this.button5);
            this.groupBox2.Controls.Add(this.Dekodiraj);
            this.groupBox2.Controls.Add(this.numericUpDown1);
            this.groupBox2.Controls.Add(this.numericUpDown2);
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Controls.Add(this.label9);
            this.groupBox2.Controls.Add(this.button2);
            this.groupBox2.Location = new System.Drawing.Point(22, 18);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(203, 147);
            this.groupBox2.TabIndex = 0;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Dekoder";
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(6, 63);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(95, 26);
            this.button2.TabIndex = 4;
            this.button2.Text = "Učitaj YUV";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.Location = new System.Drawing.Point(132, 14);
            this.numericUpDown1.Maximum = new decimal(new int[] {
            5000,
            0,
            0,
            0});
            this.numericUpDown1.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size(52, 20);
            this.numericUpDown1.TabIndex = 1;
            this.numericUpDown1.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // numericUpDown2
            // 
            this.numericUpDown2.Location = new System.Drawing.Point(132, 38);
            this.numericUpDown2.Maximum = new decimal(new int[] {
            5000,
            0,
            0,
            0});
            this.numericUpDown2.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDown2.Name = "numericUpDown2";
            this.numericUpDown2.Size = new System.Drawing.Size(52, 20);
            this.numericUpDown2.TabIndex = 3;
            this.numericUpDown2.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(20, 18);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(68, 13);
            this.label8.TabIndex = 0;
            this.label8.Text = "Početni frejm";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(20, 42);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(67, 13);
            this.label9.TabIndex = 2;
            this.label9.Text = "Završni frejm";
            // 
            // Dekodiraj
            // 
            this.Dekodiraj.Location = new System.Drawing.Point(6, 94);
            this.Dekodiraj.Name = "Dekodiraj";
            this.Dekodiraj.Size = new System.Drawing.Size(192, 24);
            this.Dekodiraj.TabIndex = 5;
            this.Dekodiraj.Text = "Pokreni dekoder";
            this.Dekodiraj.UseVisualStyleBackColor = true;
            this.Dekodiraj.Click += new System.EventHandler(this.Dekodiraj_Click);
            // 
            // button4
            // 
            this.button4.Location = new System.Drawing.Point(107, 213);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(91, 26);
            this.button4.TabIndex = 15;
            this.button4.Text = "Odaberi izlaz";
            this.button4.UseVisualStyleBackColor = true;
            this.button4.Click += new System.EventHandler(this.button4_Click);
            // 
            // button5
            // 
            this.button5.Location = new System.Drawing.Point(107, 63);
            this.button5.Name = "button5";
            this.button5.Size = new System.Drawing.Size(91, 26);
            this.button5.TabIndex = 16;
            this.button5.Text = "Odaberi izlaz";
            this.button5.UseVisualStyleBackColor = true;
            this.button5.Click += new System.EventHandler(this.button5_Click);
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(20, 125);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(40, 13);
            this.label10.TabIndex = 16;
            this.label10.Text = "Status:";
            // 
            // statusDekoder
            // 
            this.statusDekoder.AutoSize = true;
            this.statusDekoder.Location = new System.Drawing.Point(67, 125);
            this.statusDekoder.Name = "statusDekoder";
            this.statusDekoder.Size = new System.Drawing.Size(46, 13);
            this.statusDekoder.TabIndex = 17;
            this.statusDekoder.Text = "Čekanje";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(907, 476);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this._podaci);
            this.Name = "Form1";
            this.Text = "H.264";
            ((System.ComponentModel.ISupportInitialize)(this.FrameStart)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.FrameEnd)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._qp)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._podaci)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._velicinaProzora)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._toleriranaGreska)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.NumericUpDown FrameStart;
        private System.Windows.Forms.NumericUpDown FrameEnd;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label _textStatus;
        private System.Windows.Forms.Button Kodiraj;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        public System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown _qp;
        private System.Windows.Forms.DataGridView _podaci;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.NumericUpDown _velicinaProzora;
        public System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown _toleriranaGreska;
        public System.Windows.Forms.Label label6;
        private System.Windows.Forms.CheckBox _osnovnoInter;
        private System.Windows.Forms.GroupBox groupBox1;
        public System.Windows.Forms.Label label7;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button Dekodiraj;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
        private System.Windows.Forms.NumericUpDown numericUpDown2;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button4;
        private System.Windows.Forms.Button button5;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label statusDekoder;

    }
}

