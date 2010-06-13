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
            ((System.ComponentModel.ISupportInitialize)(this.FrameStart)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.FrameEnd)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._qp)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._podaci)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._velicinaProzora)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._toleriranaGreska)).BeginInit();
            this.SuspendLayout();
            // 
            // FrameStart
            // 
            this.FrameStart.Location = new System.Drawing.Point(139, 168);
            this.FrameStart.Maximum = new decimal(new int[] {
            1000,
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
            this.FrameStart.TabIndex = 0;
            this.FrameStart.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.FrameStart.KeyUp += new System.Windows.Forms.KeyEventHandler(this.FrameStart_KeyUp);
            // 
            // FrameEnd
            // 
            this.FrameEnd.Location = new System.Drawing.Point(139, 204);
            this.FrameEnd.Maximum = new decimal(new int[] {
            1000,
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
            this.FrameEnd.TabIndex = 1;
            this.FrameEnd.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(27, 170);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(68, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Početni frejm";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(28, 206);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(67, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Završni frejm";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(40, 273);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(40, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Status:";
            // 
            // _textStatus
            // 
            this._textStatus.AutoSize = true;
            this._textStatus.Location = new System.Drawing.Point(87, 273);
            this._textStatus.Name = "_textStatus";
            this._textStatus.Size = new System.Drawing.Size(46, 13);
            this._textStatus.TabIndex = 5;
            this._textStatus.Text = "Čekanje";
            // 
            // Kodiraj
            // 
            this.Kodiraj.Location = new System.Drawing.Point(125, 230);
            this.Kodiraj.Name = "Kodiraj";
            this.Kodiraj.Size = new System.Drawing.Size(84, 30);
            this.Kodiraj.TabIndex = 6;
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
            this.label4.Location = new System.Drawing.Point(28, 139);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(22, 13);
            this.label4.TabIndex = 8;
            this.label4.Text = "QP";
            // 
            // _qp
            // 
            this._qp.Location = new System.Drawing.Point(139, 137);
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
            this._qp.TabIndex = 9;
            this._qp.Value = new decimal(new int[] {
            12,
            0,
            0,
            0});
            // 
            // _podaci
            // 
            this._podaci.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this._podaci.Location = new System.Drawing.Point(215, 15);
            this._podaci.Name = "_podaci";
            this._podaci.Size = new System.Drawing.Size(680, 277);
            this._podaci.TabIndex = 10;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(21, 230);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(87, 30);
            this.button1.TabIndex = 11;
            this.button1.Text = "Učitaj Y4M";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // _velicinaProzora
            // 
            this._velicinaProzora.Location = new System.Drawing.Point(139, 107);
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
            this._velicinaProzora.TabIndex = 13;
            this._velicinaProzora.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(28, 109);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(82, 13);
            this.label5.TabIndex = 12;
            this.label5.Text = "Veličina prozora";
            // 
            // _toleriranaGreska
            // 
            this._toleriranaGreska.Location = new System.Drawing.Point(139, 78);
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
            this._toleriranaGreska.TabIndex = 15;
            this._toleriranaGreska.Value = new decimal(new int[] {
            3,
            0,
            0,
            0});
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(28, 80);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(89, 13);
            this.label6.TabIndex = 14;
            this.label6.Text = "Tolerirana greška";
            // 
            // _osnovnoInter
            // 
            this._osnovnoInter.AutoSize = true;
            this._osnovnoInter.Location = new System.Drawing.Point(31, 46);
            this._osnovnoInter.Name = "_osnovnoInter";
            this._osnovnoInter.Size = new System.Drawing.Size(152, 17);
            this._osnovnoInter.TabIndex = 16;
            this._osnovnoInter.Text = "Osnovno Inter predviđanje";
            this._osnovnoInter.UseVisualStyleBackColor = true;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(907, 410);
            this.Controls.Add(this._osnovnoInter);
            this.Controls.Add(this._toleriranaGreska);
            this.Controls.Add(this.label6);
            this.Controls.Add(this._velicinaProzora);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.button1);
            this.Controls.Add(this._podaci);
            this.Controls.Add(this._qp);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.Kodiraj);
            this.Controls.Add(this._textStatus);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.FrameEnd);
            this.Controls.Add(this.FrameStart);
            this.Name = "Form1";
            this.Text = "H.264";
            ((System.ComponentModel.ISupportInitialize)(this.FrameStart)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.FrameEnd)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._qp)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._podaci)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._velicinaProzora)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._toleriranaGreska)).EndInit();
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

    }
}

