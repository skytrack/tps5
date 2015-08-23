function TooltipsArrayGL(options)
{
    console.log('Tooltip array created');

	var default_options = {
		z_index: 6
	};

	for (var key in default_options) {
		if (typeof options[key] != typeof default_options[key]) {
			options[key] = default_options[key];
		}
	}

	this.options	= options;
	this.gl			= null;

	var canvas		= document.createElement("canvas");
	
	var context		= canvas.getContext("2d");
	                                       
	this.canvas		= canvas;
	this.context	= context;
	this.tooltips	= [];
	this.font		= '14pt Calibri';
	this.margin		= 5;
};

TooltipsArrayGL.prototype.restore = function(gl) {

	console.log('tooltip array objects recreated');

	this.gl = gl;

	// DRAW SHADER
	var drawVertexShader = gl.createShader(gl.VERTEX_SHADER);
	this.drawVertexShader = drawVertexShader;
	
	gl.shaderSource(drawVertexShader,	"\
									precision mediump float;\
									attribute vec2 a_position;\
									attribute vec2 a_normal;\
									attribute vec2 a_textureCoordinate;\
									uniform mat4 u_mv_matrix;\
									uniform mat4 u_proj_matrix;\
									varying vec2 v_textureCoordinate;\
									void main(void) {\
										gl_Position = u_proj_matrix * (u_mv_matrix * vec4(a_position, 0, 1) + vec4(a_normal, 0, 0));\
										v_textureCoordinate = a_textureCoordinate;\
									}");

	gl.compileShader(drawVertexShader);

	if (!gl.getShaderParameter(drawVertexShader, gl.COMPILE_STATUS)) {
		console.log('Tooltip draw vertex shader ' + gl.getShaderInfoLog(drawVertexShader));
	}

	var drawFragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	this.drawFragmentShader = drawFragmentShader;

	gl.shaderSource(drawFragmentShader, "\
									precision mediump float;\
									uniform sampler2D u_sampler;\
									varying vec2 v_textureCoordinate;\
									void main(void) {\
										gl_FragColor = texture2D(u_sampler, v_textureCoordinate);\
									}");

	gl.compileShader(drawFragmentShader);

	if (!gl.getShaderParameter(drawFragmentShader, gl.COMPILE_STATUS)) {
		console.log('Tooltip draw fragment shader ' + gl.getShaderInfoLog(drawFragmentShader));
	}

	var drawShaderProgram = gl.createProgram();
	this.drawShaderProgram = drawShaderProgram;

	gl.attachShader(drawShaderProgram, drawVertexShader);
	gl.attachShader(drawShaderProgram, drawFragmentShader);
	gl.linkProgram(drawShaderProgram);

	if (!gl.getProgramParameter(drawShaderProgram, gl.LINK_STATUS))
		console.log("Unable to link tooltip_array draw program");

	gl.useProgram(drawShaderProgram);
	
	drawShaderProgram.posAttrib					= gl.getAttribLocation(drawShaderProgram,  "a_position");
	drawShaderProgram.normalAttrib				= gl.getAttribLocation(drawShaderProgram,  "a_normal");
	drawShaderProgram.textureAttrib				= gl.getAttribLocation(drawShaderProgram,  "a_textureCoordinate");
	drawShaderProgram.modelviewMatrixUniform	= gl.getUniformLocation(drawShaderProgram, "u_mv_matrix");
	drawShaderProgram.projMatrixUniform			= gl.getUniformLocation(drawShaderProgram, "u_proj_matrix");
	drawShaderProgram.samplerUniform			= gl.getUniformLocation(drawShaderProgram, "u_sampler");

	// SELECT SHADER
	var selectVertexShader = gl.createShader(gl.VERTEX_SHADER);
	this.selectVertexShader = selectVertexShader;
	
	gl.shaderSource(selectVertexShader,	"\
									precision mediump float;\
									attribute vec2 a_position;\
									attribute vec2 a_normal;\
									attribute vec2 a_id;\
									attribute vec2 a_textureCoordinate;\
									uniform mat4 u_mv_matrix;\
									uniform mat4 u_proj_matrix;\
									varying vec2 v_id;\
									varying vec2 v_textureCoordinate;\
									void main(void) {\
										gl_Position = u_proj_matrix * (u_mv_matrix * vec4(a_position, 0, 1) + vec4(a_normal, 0, 0));\
										v_id = a_id;\
										v_textureCoordinate = a_textureCoordinate;\
									}");

	gl.compileShader(selectVertexShader);

	if (!gl.getShaderParameter(selectVertexShader, gl.COMPILE_STATUS)) {
		console.log('Tooltip select vertex shader ' + gl.getShaderInfoLog(selectVertexShader));
	}

	var selectFragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	this.selectFragmentShader = selectFragmentShader;

	gl.shaderSource(selectFragmentShader, "\
									precision mediump float;\
									uniform sampler2D u_sampler;\
									varying vec2 v_id;\
									varying vec2 v_textureCoordinate;\
									void main(void) {\
										vec4 color = texture2D(u_sampler, v_textureCoordinate);\
										gl_FragColor = (color.a == 1.0) ? vec4(v_id, 0.0, 1.0) : vec4(0);\
									}");

	gl.compileShader(selectFragmentShader);

	if (!gl.getShaderParameter(selectFragmentShader, gl.COMPILE_STATUS)) {
		console.log('Tooltip select fragment shader ' + gl.getShaderInfoLog(selectFragmentShader));
	}

	var selectShaderProgram = gl.createProgram();
	this.selectShaderProgram = selectShaderProgram;

	gl.attachShader(selectShaderProgram, selectVertexShader);
	gl.attachShader(selectShaderProgram, selectFragmentShader);
	gl.linkProgram(selectShaderProgram);

	if (!gl.getProgramParameter(selectShaderProgram, gl.LINK_STATUS))
		console.log("Unable to link tooltip_array select program");

	gl.useProgram(selectShaderProgram);
	
	selectShaderProgram.posAttrib				= gl.getAttribLocation(selectShaderProgram,  "a_position");
	selectShaderProgram.normalAttrib			= gl.getAttribLocation(selectShaderProgram,  "a_normal");
	selectShaderProgram.idAttrib				= gl.getAttribLocation(selectShaderProgram,  "a_id");
	selectShaderProgram.modelviewMatrixUniform	= gl.getUniformLocation(selectShaderProgram, "u_mv_matrix");
	selectShaderProgram.projMatrixUniform		= gl.getUniformLocation(selectShaderProgram, "u_proj_matrix");
	selectShaderProgram.textureAttrib			= gl.getAttribLocation(selectShaderProgram,  "a_textureCoordinate");
	selectShaderProgram.samplerUniform			= gl.getUniformLocation(selectShaderProgram, "u_sampler");

	this.createDynamicObjects();
	this.createTextures(gl);
};

TooltipsArrayGL.prototype.lost = function() {
	this.gl = null;
};

TooltipsArrayGL.prototype.createDynamicObjects = function() {

	var gl = this.gl;

	console.log('tooltip array dynamic objects recreated for ' + this.tooltips.length + ' tooltips');

	if ((gl === null)||(this.tooltips.length == 0))
		return;

	// POSITIONS
	var position_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, position_buffer);

	gl.bufferData(gl.ARRAY_BUFFER, this.position_data, gl.STATIC_DRAW);

	this.position_buffer = position_buffer;

	// NORMAL
	var normal_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, normal_buffer);

	gl.bufferData(gl.ARRAY_BUFFER, this.normal_data, gl.STATIC_DRAW);

	this.normal_buffer = normal_buffer;

	// TEXTURE COORDS
	var texcoord_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, texcoord_buffer);

	gl.bufferData(gl.ARRAY_BUFFER, this.texcoord_data, gl.STATIC_DRAW);

	this.texcoord_buffer = texcoord_buffer;

	// INDEX
	var index_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, index_buffer);

	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this.index_data, gl.STATIC_DRAW);

	this.index_buffer = index_buffer;

	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
};

TooltipsArrayGL.prototype.cleanup_dynamic = function() {

	var gl = this.gl;
		
	this.elements_count = 0;

	if (gl === null)
		return;

	if (typeof this.position_buffer == 'object') {
		gl.deleteBuffer(this.position_buffer);
		delete this.position_buffer;
	}
	if (typeof this.normal_buffer == 'object') {
		gl.deleteBuffer(this.normal_buffer);
		delete this.normal_buffer;
	}
	if (typeof this.texcoord_buffer == 'object') {
		gl.deleteBuffer(this.texcoord_buffer);
		delete this.texcoord_buffer;
	}
	if (typeof this.index_buffer == 'object') {
    	gl.deleteBuffer(this.index_buffer);
		delete this.index_buffer;
	}

	var tooltips	= this.tooltips;
	var len			= tooltips.length;

	for (var i = 0; i < len; i++) {

		var tooltip = tooltips[i];

		if (typeof tooltip.texture == 'object') {
			gl.deleteTexture(tooltip.texture);
	    	delete tooltip.texture;
		}
	}
};

TooltipsArrayGL.prototype.cleanup = function() {

	var gl = this.gl;
		
	if (gl === null)
		return;

	if (typeof this.drawShaderProgram == 'object') {
		gl.deleteProgram(this.drawShaderProgram);
		delete this.drawShaderProgram;
	}

	if (typeof this.drawVertexShader == 'object') {
		gl.deleteShader(this.drawVertexShader);
		delete this.drawVertexShader;
	}
	if (typeof this.drawFragmentShader == 'object') {
		gl.deleteShader(this.drawFragmentShader);
		delete this.drawFragmentShader;
	}

	if (typeof this.selectShaderProgram == 'object') {
		gl.deleteProgram(this.selectShaderProgram);
		delete this.selectShaderProgram;
	}

	if (typeof this.selectVertexShader == 'object') {
		gl.deleteShader(this.selectVertexShader);
		delete this.selectVertexShader;
	}
	if (typeof this.selectFragmentShader == 'object') {
		gl.deleteShader(this.selectFragmentShader);
		delete this.selectFragmentShader;
	}

	if (typeof this.texture == 'object') {
		gl.deleteTexture(this.texture);
		delete this.texture;
		console.log('texture destroyed');
	}

	this.cleanup_dynamic(gl);
};                                              

TooltipsArrayGL.prototype.addTooltip = function(tooltip) {

	var tooltips = this.tooltips;

	tooltips.push(tooltip);

	if (this.updating != true)
		this.endUpdate();

	return this;
};

TooltipsArrayGL.prototype.removeTooltip = function(tooltip) {

	var tooltips = this.tooltips;

	var i;
	while ((i = tooltips.indexOf(tooltip)) != -1) {
		tooltips.splice(i, 1);
	}

	if (this.updating !== true)
		this.endUpdate();

	return this;
};

TooltipsArrayGL.prototype.removeAll = function() {

	this.tooltips = [];
	this.cleanup_dynamic();

	return this;
};

TooltipsArrayGL.prototype.createTextures = function() {

	var gl 			= this.gl;
	var canvas		= this.canvas;
	var context		= this.context;
	var tooltips	= this.tooltips;
	var len			= tooltips.length;

	if (this.gl === null)
		return;

	gl.activeTexture(gl.TEXTURE0);

	for (var i = 0; i < len; i++) {

		var tooltip = tooltips[i];
	
	    if (typeof tooltip.texture != 'object') {

			context.font			= this.font;

			var txtWidth 	= context.measureText(tooltip.text).width + this.margin * 2 + 2;

			var texWidth	= 2;
			while (texWidth < txtWidth)
				texWidth = texWidth * 2;

			canvas.width	= texWidth;
			canvas.height	= 32;
	
			context.beginPath();
			context.moveTo(1,1);
			context.lineTo(txtWidth - 1, 1);
			context.lineTo(txtWidth - 1, 24);
			context.lineTo(16, 24);
			context.lineTo(12, 31);
			context.lineTo(8, 24);
			context.lineTo(1, 24);
			context.lineTo(1, 1);

			context.fillStyle = 'white';
			context.fill();

			context.lineWidth = 2;
			context.strokeStyle = '#' + tooltip.color;
			context.stroke();
  
			context.fillStyle = 'black';

			context.textBaseline	= 'middle';
			context.font			= this.font;
			context.fillText(tooltip.text, 1 + this.margin, 13);

			var texture = gl.createTexture();

			gl.bindTexture(gl.TEXTURE_2D, texture);
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);

			tooltip.texture	= texture;
			tooltip.width	= txtWidth;
			tooltip.height	= canvas.height;
			tooltip.texWidth= texWidth;
			tooltip.texHeight = canvas.height;
			tooltip.offset_x	= (tooltip.width / 2) - 12;
			tooltip.offset_y	= tooltip.height / 2;
		}        		
	}

	gl.bindTexture(gl.TEXTURE_2D, null);
};

TooltipsArrayGL.prototype.render = function(proj, modelView) {

	var gl = this.gl;
	var tooltips = this.tooltips;
	var len = tooltips.length;    	

	if ((typeof gl != 'object')||(len === 0)||(this.updating === true))  
		return;                                           

	var drawShaderProgram = this.drawShaderProgram;

	if (typeof drawShaderProgram != 'object')
		return;

	gl.useProgram(drawShaderProgram);

	gl.uniform1i(drawShaderProgram.samplerUniform, 0);
	gl.uniformMatrix4fv(drawShaderProgram.modelviewMatrixUniform, false, modelView); 
	gl.uniformMatrix4fv(drawShaderProgram.projMatrixUniform, false, proj);

	gl.enableVertexAttribArray(drawShaderProgram.posAttrib);
	gl.enableVertexAttribArray(drawShaderProgram.normalAttrib);
	gl.enableVertexAttribArray(drawShaderProgram.textureAttrib);    

	gl.bindBuffer(gl.ARRAY_BUFFER, this.position_buffer);
    gl.vertexAttribPointer(drawShaderProgram.posAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.bindBuffer(gl.ARRAY_BUFFER, this.normal_buffer);
    gl.vertexAttribPointer(drawShaderProgram.normalAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.bindBuffer(gl.ARRAY_BUFFER, this.texcoord_buffer);
    gl.vertexAttribPointer(drawShaderProgram.textureAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.index_buffer);

	gl.activeTexture(gl.TEXTURE0);

	for (var i = 0; i < len; i++ ) {
		gl.bindTexture(gl.TEXTURE_2D, tooltips[i].texture);
		gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, i * 6 * 2); 
	}

	gl.disableVertexAttribArray(drawShaderProgram.posAttrib);
	gl.disableVertexAttribArray(drawShaderProgram.normalAttrib);
	gl.disableVertexAttribArray(drawShaderProgram.textureAttrib);

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);    
	gl.bindTexture(gl.TEXTURE_2D, null);
};
                      
TooltipsArrayGL.prototype.lookup = function(proj, modelView, id_buffer) {

	var gl = this.gl;
	var tooltips = this.tooltips;
	var len = tooltips.length;

	if ((typeof gl != 'object')||(len === 0)||(this.updating === true))
		return;

	var selectShaderProgram = this.selectShaderProgram;

	if (typeof selectShaderProgram != 'object')
		return;

	gl.useProgram(selectShaderProgram);

	gl.uniform1i(selectShaderProgram.samplerUniform, 0);
	gl.uniformMatrix4fv(selectShaderProgram.modelviewMatrixUniform, false, modelView); 
	gl.uniformMatrix4fv(selectShaderProgram.projMatrixUniform, false, proj);

	gl.enableVertexAttribArray(selectShaderProgram.posAttrib);
	gl.enableVertexAttribArray(selectShaderProgram.normalAttrib);
	gl.enableVertexAttribArray(selectShaderProgram.idAttrib);
	gl.enableVertexAttribArray(selectShaderProgram.textureAttrib);    

	gl.bindBuffer(gl.ARRAY_BUFFER, this.position_buffer);
    gl.vertexAttribPointer(selectShaderProgram.posAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.bindBuffer(gl.ARRAY_BUFFER, this.normal_buffer);
    gl.vertexAttribPointer(selectShaderProgram.normalAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.bindBuffer(gl.ARRAY_BUFFER, id_buffer);
	gl.vertexAttribPointer(selectShaderProgram.idAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.bindBuffer(gl.ARRAY_BUFFER, this.texcoord_buffer);
    gl.vertexAttribPointer(selectShaderProgram.textureAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.index_buffer);

	gl.activeTexture(gl.TEXTURE0);

	for (var i = 0; i < len; i++ ) {
		gl.bindTexture(gl.TEXTURE_2D, tooltips[i].texture);
		gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, i * 6 * 2); 
	}

	gl.disableVertexAttribArray(selectShaderProgram.posAttrib);
	gl.disableVertexAttribArray(selectShaderProgram.normalAttrib);
	gl.disableVertexAttribArray(selectShaderProgram.idAttrib);    
	gl.disableVertexAttribArray(selectShaderProgram.textureAttrib);    

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);    
	gl.bindTexture(gl.TEXTURE_2D, null);
};

TooltipsArrayGL.prototype.getPopup = function(vertex_index) {

	var tooltip = this.tooltips[Math.floor(vertex_index / 4)];

	return tooltip.popup_text();
};                  

TooltipsArrayGL.prototype.beginUpdate = function() {
	
	this.updating = true;

	return this;
};

TooltipsArrayGL.prototype.endUpdate = function() {

	this.updating		= false;

	var tooltips		= this.tooltips;

	var tooltips_count	= tooltips.length;

	this.cleanup_dynamic();

	this.createTextures(this.gl);

	var position_index	= 0;
	var normal_index	= 0;
	var texcoord_index	= 0;
	var point_index		= 0;
	var index_index		= 0;

	var position_data	= new Float32Array(tooltips_count * 4 * 2);
	var normal_data		= new Float32Array(tooltips_count * 4 * 2);
	var texcoord_data	= new Float32Array(tooltips_count * 4 * 2);
	var index_data		= new Uint16Array(tooltips_count * 6);

	for (var i = 0; i < tooltips_count; i++) {

		var tooltip = tooltips[i];

		var len		= Math.sqrt(tooltip.width * tooltip.width + tooltip.height * tooltip.height) / 2;
		var angle	= Math.atan(tooltip.height / tooltip.width) * 180 / Math.PI;

		var x = 6378137 * tooltip.lng * (Math.PI/180);
		var y = 6378137 * Math.log(Math.tan(Math.PI/4+tooltip.lat*(Math.PI/180)/2)); 

		position_data[position_index++] = x;
		position_data[position_index++] = y;
		position_data[position_index++] = x;
		position_data[position_index++] = y;
		position_data[position_index++] = x;
		position_data[position_index++] = y;
		position_data[position_index++] = x;
		position_data[position_index++] = y;

		var x1 = ( len * Math.cos((angle) * Math.PI / 180)); 
		var y1 = (-len * Math.sin((angle) * Math.PI / 180));
		var x2 = ( len * Math.cos((-angle) * Math.PI / 180)); 
		var y2 = (-len * Math.sin((-angle) * Math.PI / 180));

		normal_data[normal_index++] = x1 + tooltip.offset_x;
		normal_data[normal_index++] = y1 + tooltip.offset_y;
		normal_data[normal_index++] = x2 + tooltip.offset_x;
		normal_data[normal_index++] = y2 + tooltip.offset_y;
		normal_data[normal_index++] = -x1 + tooltip.offset_x;
		normal_data[normal_index++] = -y1 + tooltip.offset_y;
		normal_data[normal_index++] = -x2 + tooltip.offset_x;
		normal_data[normal_index++] = -y2 + tooltip.offset_y;

		texcoord_data[texcoord_index++] = tooltip.width / tooltip.texWidth;
		texcoord_data[texcoord_index++] = tooltip.height / tooltip.texHeight;
		texcoord_data[texcoord_index++] = tooltip.width / tooltip.texWidth;
		texcoord_data[texcoord_index++] = 0;
		texcoord_data[texcoord_index++] = 0;
		texcoord_data[texcoord_index++] = 0;
		texcoord_data[texcoord_index++] = 0;
		texcoord_data[texcoord_index++] = tooltip.height / tooltip.texHeight;

		var base_point = i * 4;

		index_data[index_index++] = base_point + 0;
		index_data[index_index++] = base_point + 1;
		index_data[index_index++] = base_point + 2;
		index_data[index_index++] = base_point + 0;
		index_data[index_index++] = base_point + 2;
		index_data[index_index++] = base_point + 3;
	}

	this.position_data	= position_data;
	this.normal_data	= normal_data;
	this.texcoord_data	= texcoord_data;
	this.index_data		= index_data;

	this.createDynamicObjects();

	return this;
};