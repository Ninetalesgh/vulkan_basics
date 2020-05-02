//thanks to https://developer.mozilla.org/ and https://youtu.be/oDiSqQT_szo
//for their amazing WebGL tutorials

var squareRotation = 0.0;

  // Vertex shader program

const vsSource = `
    attribute vec4 aVertexPosition;
    attribute vec2 aTextureCoord;

    uniform mat4 uModelViewMatrix;
    uniform mat4 uProjectionMatrix;

    varying highp vec2 vTextureCoord;

    void main(void) {
      gl_Position = uProjectionMatrix * uModelViewMatrix * aVertexPosition;
      vTextureCoord = aTextureCoord;
    }
  `;

  const fsSource = `
  precision mediump float;
  varying highp vec2 vTextureCoord;

  uniform sampler2D uSampler;

  float fade(float fWidth)
  {
    float x = vTextureCoord.x;
    float y = vTextureCoord.y;

    float denominator = (fWidth*fWidth - fWidth);
    float xFade = min(1.5, (x*x-x)/denominator);
    float yFade = min(1.5, (y*y-y)/denominator);
    return min(1.0, xFade * yFade);
  }

  void main(void) {
    gl_FragColor = texture2D(uSampler, vTextureCoord);
    gl_FragColor.w *= fade(0.1);
  }
`;


var state = {
  canvas: null,
  gl: null,
  programInfo: 
  {
    program: null,
    attribLocations: {},
    uniformLocations: {},
  },
  app: 
  {  
    animations: [],
    camera: 
    {
      projectionMatrix: null,
    },
    objects: [],
  },
};

function createAnimation(tickFunction)
{

  //speed
  //keyPos []
  //startTrigger
  //tickCallback

  return {
    isActive: false,
    tick: tickFunction,
    t: 0.0,
    speed: 1.0,

  };
}

var POS_OFFSET = -1.5;

function createObject(texture)
{
//load image 

  // Set the drawing position to the "identity" point, which is
  // the center of the scene.
  const modelViewMatrix = mat4.create();

  // Now move the drawing position a bit to where we want to
  // start drawing the square.

  mat4.translate(modelViewMatrix,     // destination matrix
                  modelViewMatrix,     // matrix to translate
                  [POS_OFFSET, 0.0, -6.0]);  // amount to translate

  POS_OFFSET += 3.0;

 // mat4.rotate(modelViewMatrix,  // destination matrix
 //             modelViewMatrix,  // matrix to rotate
 //             squareRotation,   // amount to rotate in radians
 //            [0, 0.3, 0.707]);       // axis to rotate around

  return {
    mvm: modelViewMatrix,
    image: texture,
    animation: null,
  };
}

//
// creates a shader of the given type, uploads the source and
// compiles it.
//
function loadShader(gl, type, source) {
  const shader = gl.createShader(type);

  // Send the source to the shader object

  gl.shaderSource(shader, source);

  // Compile the shader program

  gl.compileShader(shader);

  // See if it compiled successfully

  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      alert('An error occurred compiling the shaders: ' + gl.getShaderInfoLog(shader));
      gl.deleteShader(shader);
      return null;
  }

  return shader;
}

//
// Initialize a shader program, so WebGL knows how to draw our data
//
function initShaderProgram(gl, vsSource, fsSource) {
  const vertexShader = loadShader(gl, gl.VERTEX_SHADER, vsSource);
  const fragmentShader = loadShader(gl, gl.FRAGMENT_SHADER, fsSource);

  // Create the shader program

  const shaderProgram = gl.createProgram();
  gl.attachShader(shaderProgram, vertexShader);
  gl.attachShader(shaderProgram, fragmentShader);
  gl.linkProgram(shaderProgram);

  // If creating the shader program failed, alert

  if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
    alert('Unable to initialize the shader program: ' + gl.getProgramInfoLog(shaderProgram));
    return null;
  }

  return shaderProgram;
}


function isPowerOf2(value) {
  return (value & (value - 1)) == 0;
}
//
// Initialize a texture and load an image.
// When the image finished loading copy it into the texture.
//
function loadTexture(gl, url) {
  const texture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, texture);

  // Because images have to be download over the internet
  // they might take a moment until they are ready.
  // Until then put a single pixel in the texture so we can
  // use it immediately. When the image has finished downloading
  // we'll update the texture with the contents of the image.
  const level = 0;
  const internalFormat = gl.RGBA;
  const width = 1;
  const height = 1;
  const border = 0;
  const srcFormat = gl.RGBA;
  const srcType = gl.UNSIGNED_BYTE;
  const pixel = new Uint8Array([0, 0, 255, 255]);  // opaque blue
  gl.texImage2D(gl.TEXTURE_2D, level, internalFormat,
                width, height, border, srcFormat, srcType,
                pixel);

  const image = new Image();
  image.onload = function() {
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, level, internalFormat,
                  srcFormat, srcType, image);

    // WebGL1 has different requirements for power of 2 images
    // vs non power of 2 images so check if the image is a
    // power of 2 in both dimensions.
    if (isPowerOf2(image.width) && isPowerOf2(image.height)) {
       // Yes, it's a power of 2. Generate mips.
       gl.generateMipmap(gl.TEXTURE_2D);
    } else {
       // No, it's not a power of 2. Turn off mips and set
       // wrapping to clamp to edge
       gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
       gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
       gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    }
  };
  image.src = url;
  return texture;
}

function initState()
{
  const canvas = document.querySelector("#glCanvas");
  // Initialize the GL context
  const gl = canvas.getContext("webgl", {premultipliedAlpha: false });

  // Only continue if WebGL is available and working
  if (gl === null) {
    alert("Unable to initialize WebGL. Your browser or machine may not support it.");
    return false;
  }

  function resizer()
  {
    //resize canvas
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);

    //recalculate projection matrix for the camera
    const fieldOfView = 0.25 * Math.PI;
    const aspect = gl.canvas.clientWidth / gl.canvas.clientHeight;
    const zNear = 0.1;
    const zFar = 100.0;
    const projectionMatrix = mat4.create();
    mat4.perspective(projectionMatrix, fieldOfView, aspect, zNear, zFar);
  
    state.app.camera.projectionMatrix = projectionMatrix;

    console.log('resized');
  }
  window.addEventListener('resize', resizer);
  resizer();   

  const shaderProgram = initShaderProgram(gl, vsSource, fsSource);
  const programInfo = {
     program: shaderProgram,
     attribLocations: {
       vertexPosition: gl.getAttribLocation(shaderProgram, 'aVertexPosition'),
       textureCoord: gl.getAttribLocation(shaderProgram, 'aTextureCoord'),
     },
     uniformLocations: {
       projectionMatrix: gl.getUniformLocation(shaderProgram, 'uProjectionMatrix'),
       modelViewMatrix: gl.getUniformLocation(shaderProgram, 'uModelViewMatrix'),
       uSampler: gl.getUniformLocation(shaderProgram, 'uSampler'),
     },
   };

   function onMouseDown(event)
   {
     var x = event.clientX, y = event.clientY;
     var midX = canvas.width/2, midY = canvas.height/2;
     var rect = event.target.getBoundingClientRect();
     x = ((x - rect.left) - midX) / midX;
     y = ((y - rect.top) - midY) / midY;
    //ADD FUNCTION CALL TO f(x,y)
     console.log("mouse: " + x + "  " + y);
   }

   canvas.addEventListener('mousedown', function(event) { onMouseDown(event); } );


  state.canvas = canvas;
  state.gl = gl;
  state.programInfo = programInfo;

  return true;
}

function initApp()
{


  return true;
}


function initBuffers(gl) {

  const positions = [
    -1.0,  1.0, 0.0,
     1.0,  1.0, 0.0,
    -1.0, -1.0, 0.0,
     1.0, -1.0, 0.0,
    ];
  const positionBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
  gl.bufferData(gl.ARRAY_BUFFER,
                new Float32Array(positions),
                gl.STATIC_DRAW);


  const indices = [0,  1,  2, 2,  1,  3,];
  const indexBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,
      new Uint16Array(indices), gl.STATIC_DRAW);


  const textureCoordinates = [
    0.0,  0.0,
    1.0,  0.0,
    0.0,  1.0,
    1.0,  1.0,
    ];
  const textureCoordBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, textureCoordBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textureCoordinates),
                gl.STATIC_DRAW);

  return {
      position: positionBuffer,
      textureCoord: textureCoordBuffer,
      indices: indexBuffer,
  };
}

function prepareFrame(gl, programInfo, buffers) {
  gl.clearColor(0.0, 0.0, 0.0, 1.0);  // Clear to black, fully opaque

  gl.clearDepth(1.0);                 // Clear everything
  gl.enable(gl.DEPTH_TEST);           // Enable depth testing
  gl.depthFunc(gl.LEQUAL);            // Near things obscure far things
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  gl.enable(gl.BLEND);
  gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

//////////////////////////////
//THIS IS PER MODEL////////////////////
//////////////////////

  // Tell WebGL how to pull out the positions from the position
  // buffer into the vertexPosition attribute.
  {
    const numComponents = 3;  // pull out 3 values per iteration
    const type = gl.FLOAT;    // the data in the buffer is 32bit floats
    const normalize = false;  // don't normalize
    const stride = 0;         // how many bytes to get from one set of values to the next
                              // 0 = use type and numComponents above
    const offset = 0;         // how many bytes inside the buffer to start from
    gl.bindBuffer(gl.ARRAY_BUFFER, buffers.position);
    gl.vertexAttribPointer(
        programInfo.attribLocations.vertexPosition,
        numComponents,
        type,
        normalize,
        stride,
        offset);
    gl.enableVertexAttribArray(
        programInfo.attribLocations.vertexPosition);
  }

  // tell webgl how to pull out the texture coordinates from buffer
  {
    const num = 2; // every coordinate composed of 2 values
    const type = gl.FLOAT; // the data in the buffer is 32 bit float
    const normalize = false; // don't normalize
    const stride = 0; // how many bytes to get from one set to the next
    const offset = 0; // how many bytes inside the buffer to start from
    gl.bindBuffer(gl.ARRAY_BUFFER, buffers.textureCoord);
    gl.vertexAttribPointer(programInfo.attribLocations.textureCoord, num, type, normalize, stride, offset);
    gl.enableVertexAttribArray(programInfo.attribLocations.textureCoord);
  }

  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buffers.indices);

//////////////////////////
// PER MODEL END/////////////////////////
/////////////////////////

  // Tell WebGL to use our program when drawing
  gl.useProgram(programInfo.program);

  // Set the shader uniforms
  gl.uniformMatrix4fv(
      programInfo.uniformLocations.projectionMatrix,
      false,
      state.app.camera.projectionMatrix);
}

function renderObject(object)
{
  const gl = state.gl;
  const programInfo = state.programInfo;

  gl.uniformMatrix4fv(
      programInfo.uniformLocations.modelViewMatrix,
      false,
      object.mvm);

  gl.activeTexture(gl.TEXTURE0);
  gl.bindTexture(gl.TEXTURE_2D, object.image);
  gl.uniform1i(programInfo.uniformLocations.uSampler, 0);
  {
    const indexCount = 6; // DEPENDS ON MODEL
    const type = gl.UNSIGNED_SHORT;
    const offset = 0;

    gl.drawElements(gl.TRIANGLES, indexCount, type, offset);
  }
}

function updateObject(object, deltaTime)
{
 return;
 
  mat4.translate(object.mvm,     // destination matrix
                  object.mvm,     // matrix to translate
                [0.0, -deltaTime/2.0, 0.0]);  // amount to translate
}


function main() {
 
    if (!initState())
    {
      return;
    }

    if (!initApp())
    {
      return;
    }


    //APP CODE  //MOVE THIS TO INITAPP
/////////////////
    const buffers = initBuffers(state.gl); 
    
    const texture = loadTexture(state.gl, 'assets/debug_color_01.png'); 
    const obj = createObject(texture);
    const obj2 = createObject(texture);
    var objects = [ obj, obj2 ];

////////////////////////

    var then = 0;
    function render(now) {
      now *= 0.001;  // convert to seconds
      const deltaTime = now - then;
      then = now;
  
      //animate objects
      objects.forEach((item) => updateObject(item, deltaTime) );

      //prepare frame
      prepareFrame(state.gl, state.programInfo, buffers);
  
      //render objects
      objects.forEach(renderObject);

      requestAnimationFrame(render);
    }

    requestAnimationFrame(render);
  }
