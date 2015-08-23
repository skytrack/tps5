function MarkerArrayGL(options) 
{
    console.log('Markers array created');

	var default_options = {
		url: '',
		width: 1,
		height: 1,
		sprites_count: 0,
		offset_x: 0,
		offset_y: 0
	};

   	for (var key in default_options) {
   		if (typeof options[key] != typeof default_options[key]) {
   			options[key] = default_options[key];
   		}
   	}

    this.sprite_number	= 0;
    this.elements_count	= 0;
    this.markers		= [];
    this.gl				= null;
   	this.options		= options;

	var image		= new Image();

    image.loaded	= false;
	image.owner		= this;
	image.onload	= this.onImageLoad;
	image.src		= this.options.url;

    this.image		= image;
};

MarkerArrayGL.prototype.onImageLoad = function() { 

	var self = this.owner;

	this.loaded = true;

	if (self.gl !== null) {
		self.createTexture();
	}
};

MarkerArrayGL.prototype.restore = function(gl) {

	console.log('markers array objects recreated');

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
		console.log('Sprite Vertex shader ' + gl.getShaderInfoLog(drawVertexShader));
	}

	var drawFragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	this.drawFragmentShader = drawFragmentShader;

	gl.shaderSource(drawFragmentShader, "\
									precision mediump float;\
									uniform sampler2D u_sampler;\
									uniform float u_offset;\
									varying vec2 v_textureCoordinate;\
									void main(void) {\
										gl_FragColor = texture2D(u_sampler, v_textureCoordinate + vec2(0, u_offset));\
									}");

	gl.compileShader(drawFragmentShader);

	if (!gl.getShaderParameter(drawFragmentShader, gl.COMPILE_STATUS)) {
		console.log('Sprite Fragment shader ' + gl.getShaderInfoLog(drawFragmentShader));
	}

	var drawShaderProgram = gl.createProgram();
	this.drawShaderProgram = drawShaderProgram;

	gl.attachShader(drawShaderProgram, drawVertexShader);
	gl.attachShader(drawShaderProgram, drawFragmentShader);
	gl.linkProgram(drawShaderProgram);

	if (!gl.getProgramParameter(drawShaderProgram, gl.LINK_STATUS))
		console.log("Unable to link markers_array draw shader");

	gl.useProgram(drawShaderProgram);
	
	drawShaderProgram.posAttrib					= gl.getAttribLocation(drawShaderProgram,  "a_position");
	drawShaderProgram.normalAttrib				= gl.getAttribLocation(drawShaderProgram,  "a_normal");
	drawShaderProgram.textureAttrib				= gl.getAttribLocation(drawShaderProgram,  "a_textureCoordinate");
	drawShaderProgram.modelviewMatrixUniform	= gl.getUniformLocation(drawShaderProgram, "u_mv_matrix");
	drawShaderProgram.projMatrixUniform			= gl.getUniformLocation(drawShaderProgram, "u_proj_matrix");
	drawShaderProgram.samplerUniform			= gl.getUniformLocation(drawShaderProgram, "u_sampler");
	drawShaderProgram.offsetUniform				= gl.getUniformLocation(drawShaderProgram, "u_offset");

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
		console.log('Select Vertex shader ' + gl.getShaderInfoLog(selectVertexShader));
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
		console.log('Select Fragment shader ' + gl.getShaderInfoLog(selectFragmentShader));
	}

	var selectShaderProgram = gl.createProgram();
	this.selectShaderProgram = selectShaderProgram;

	gl.attachShader(selectShaderProgram, selectVertexShader);
	gl.attachShader(selectShaderProgram, selectFragmentShader);
	gl.linkProgram(selectShaderProgram);

	if (!gl.getProgramParameter(selectShaderProgram, gl.LINK_STATUS))
		console.log("Unable to link markers_array select shader");

	gl.useProgram(selectShaderProgram);
	
	selectShaderProgram.posAttrib				= gl.getAttribLocation(selectShaderProgram,  "a_position");
	selectShaderProgram.normalAttrib			= gl.getAttribLocation(selectShaderProgram,  "a_normal");
	selectShaderProgram.idAttrib				= gl.getAttribLocation(selectShaderProgram,  "a_id");
	selectShaderProgram.modelviewMatrixUniform	= gl.getUniformLocation(selectShaderProgram, "u_mv_matrix");
	selectShaderProgram.projMatrixUniform		= gl.getUniformLocation(selectShaderProgram, "u_proj_matrix");
	selectShaderProgram.textureAttrib			= gl.getAttribLocation(selectShaderProgram,  "a_textureCoordinate");
	selectShaderProgram.samplerUniform			= gl.getUniformLocation(selectShaderProgram, "u_sampler");

 	if (this.image.loaded) {
		this.createTexture(gl);
	}

	this.createDynamicObjects();
};

MarkerArrayGL.prototype.lost = function() {
	this.gl = null;
};

MarkerArrayGL.prototype.createDynamicObjects = function() {

	var gl = this.gl;

	console.log('markers array dynamic objects recreated for ' + this.markers.length + ' markers');

	if ((typeof gl != 'object')||(this.markers.length == 0))
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

MarkerArrayGL.prototype.createTexture = function() 
{
	console.log('texture recreated');

	var gl = this.gl;

	var texture = gl.createTexture();

	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, texture);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.image);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);

	gl.bindTexture(gl.TEXTURE_2D, null);

	this.texture = texture;
};

MarkerArrayGL.prototype.cleanup_dynamic = function() {

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
};

MarkerArrayGL.prototype.cleanup = function() {

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

	this.cleanup_dynamic();
};                                              

MarkerArrayGL.prototype.addMarker = function(marker) {

	var markers = this.markers;
	markers.push(marker);

	if (this.updating != true)
		this.endUpdate();

	return this;
};

MarkerArrayGL.prototype.removeMarker = function(marker) {

	var markers = this.markers;

	var i;
	while ((i = markers.indexOf(marker)) != -1) {
		markers.splice(i, 1);
	}

	if (this.updating !== true)
		this.endUpdate();

	return this;
};

MarkerArrayGL.prototype.removeAll = function() {

	this.markers = [];
	this.cleanup_dynamic();

	return this;
};

MarkerArrayGL.prototype.render = function(proj, modelView) {

	var gl = this.gl;

	if ((typeof gl != 'object')||(typeof this.texture != 'object')||(this.elements_count === 0)) 
		return;                                           

	var drawShaderProgram = this.drawShaderProgram;

	if (typeof drawShaderProgram != 'object')
		return;

	this.sprite_number++;

	if (this.sprite_number >= this.options.sprites_count) {
		this.sprite_number = 0;
	}

	gl.useProgram(drawShaderProgram);

	gl.uniform1i(drawShaderProgram.samplerUniform, 0);
	gl.uniformMatrix4fv(drawShaderProgram.modelviewMatrixUniform, false, modelView); 
	gl.uniformMatrix4fv(drawShaderProgram.projMatrixUniform, false, proj);
	gl.uniform1f(drawShaderProgram.offsetUniform, (this.sprite_number * this.options.height) / this.image.height);

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
	gl.bindTexture(gl.TEXTURE_2D, this.texture);

	gl.drawElements(gl.TRIANGLES, this.elements_count, gl.UNSIGNED_SHORT, 0); 

	gl.disableVertexAttribArray(drawShaderProgram.posAttrib);
	gl.disableVertexAttribArray(drawShaderProgram.normalAttrib);
	gl.disableVertexAttribArray(drawShaderProgram.textureAttrib);

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);    
	gl.bindTexture(gl.TEXTURE_2D, null);
};
                      
MarkerArrayGL.prototype.lookup = function(proj, modelView, id_buffer) {

	var gl = this.gl;

	if ((typeof gl != 'object')||(typeof this.texture != 'object')||(this.elements_count === 0)) 
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
	gl.bindTexture(gl.TEXTURE_2D, this.texture);

	gl.drawElements(gl.TRIANGLES, this.elements_count, gl.UNSIGNED_SHORT, 0); 

	gl.disableVertexAttribArray(selectShaderProgram.posAttrib);
	gl.disableVertexAttribArray(selectShaderProgram.normalAttrib);
	gl.disableVertexAttribArray(selectShaderProgram.idAttrib);    
	gl.disableVertexAttribArray(selectShaderProgram.textureAttrib);    

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);    
	gl.bindTexture(gl.TEXTURE_2D, null);
};

MarkerArrayGL.prototype.getPopup = function(vertex_index) {

	var marker = this.markers[Math.floor(vertex_index / 4)];

	return marker.popup_text(marker.index);
};                  

MarkerArrayGL.prototype.beginUpdate = function() {
	
	this.updating = true;

	return this;
};

MarkerArrayGL.prototype.endUpdate = function() {

	this.updating		= false;

	var markers			= this.markers;

	var markers_count	= markers.length;

	this.cleanup_dynamic();

	var position_index	= 0;
	var normal_index	= 0;
	var texcoord_index	= 0;
	var point_index		= 0;
	var index_index		= 0;

	var position_data	= new Float32Array(markers_count * 4 * 2);
	var normal_data		= new Float32Array(markers_count * 4 * 2);
	var texcoord_data	= new Float32Array(markers_count * 4 * 2);
	var index_data		= new Uint16Array(markers_count * 6);

	var len				= Math.sqrt(this.options.width * this.options.width + this.options.height * this.options.height) / 2;
	var angle			= Math.atan(this.options.height / this.options.width) * 180 / Math.PI;

	for (var i = 0; i < markers_count; i++) {

		var marker = markers[i];

		var x = 6378137 * marker.lng * (Math.PI/180);
		var y = 6378137 * Math.log(Math.tan(Math.PI/4+marker.lat*(Math.PI/180)/2)); 

		position_data[position_index++] = x;
		position_data[position_index++] = y;
		position_data[position_index++] = x;
		position_data[position_index++] = y;
		position_data[position_index++] = x;
		position_data[position_index++] = y;
		position_data[position_index++] = x;
		position_data[position_index++] = y;

		var x1 = ( len * Math.cos((marker.cog + angle) * Math.PI / 180)); 
		var y1 = (-len * Math.sin((marker.cog + angle) * Math.PI / 180));
		var x2 = ( len * Math.cos((marker.cog - angle) * Math.PI / 180)); 
		var y2 = (-len * Math.sin((marker.cog - angle) * Math.PI / 180));

		normal_data[normal_index++] = x1 + this.options.offset_x;
		normal_data[normal_index++] = y1 + this.options.offset_y;
		normal_data[normal_index++] = x2 + this.options.offset_x;
		normal_data[normal_index++] = y2 + this.options.offset_y;
		normal_data[normal_index++] = -x1 + this.options.offset_x;
		normal_data[normal_index++] = -y1 + this.options.offset_y;
		normal_data[normal_index++] = -x2 + this.options.offset_x;
		normal_data[normal_index++] = -y2 + this.options.offset_y;

		texcoord_data[texcoord_index++] = this.options.width / this.image.width;
		texcoord_data[texcoord_index++] = this.options.height / this.image.height;
		texcoord_data[texcoord_index++] = this.options.width / this.image.width;
		texcoord_data[texcoord_index++] = 0;
		texcoord_data[texcoord_index++] = 0;
		texcoord_data[texcoord_index++] = 0;
		texcoord_data[texcoord_index++] = 0;
		texcoord_data[texcoord_index++] = this.options.height / this.image.height;

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

	this.elements_count = markers_count * 6;

	this.createDynamicObjects();

	return this;
};