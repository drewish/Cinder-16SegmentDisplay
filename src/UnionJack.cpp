//
//  UnionJack.cpp
//
//  Draws 16-segments displays aka "Union Jack"
//
//  Created by Andrew Morton on 11/6/14.
//

#include "UnionJack.h"

using namespace ci;
using namespace ci::geom;
using namespace std;

// Segments (order is 0-F):
//
//   *-0-*-1-*
//   |\  |  /|
//   7 A B C 2
//   |  \|/  |
//   *-8-*-9-*
//   |  /|\  |
//   6 F E D 3
//   |/  |  \|
//   *-5-*-4-*
//
// Awesome font stolen from http://www.msarnoff.org/alpha32/
// The first 32, non-printable ASCII characters are omitted.
static const unsigned int CHAR_OFFSET = 32;
static const unsigned int CHAR_LENGTH = 96;
static uint16_t charPatterns[CHAR_LENGTH] = {
    0x0000,  /*   */ 0x1822,  /* ! */ 0x0880,  /* " */ 0x4b3c,  /* # */
    0x4bbb,  /* $ */ 0xdb99,  /* % */ 0x2d79,  /* & */ 0x1000,  /* ' */
    0x3000,  /* ( */ 0x8400,  /* ) */ 0xff00,  /* * */ 0x4b00,  /* + */
    0x8000,  /* , */ 0x0300,  /* - */ 0x0020,  /* . */ 0x9000,  /* / */
    0x90ff,  /* 0 */ 0x100c,  /* 1 */ 0x0377,  /* 2 */ 0x123b,  /* 3 */
    0x038c,  /* 4 */ 0x21b3,  /* 5 */ 0x03fb,  /* 6 */ 0x000f,  /* 7 */
    0x03ff,  /* 8 */ 0x03bf,  /* 9 */ 0x0021,  /* : */ 0x8001,  /* ; */
    0x9030,  /* < */ 0x0330,  /* = */ 0x2430,  /* > */ 0x4207,  /* ? */
    0x417f,  /* @ */ 0x03cf,  /* A */ 0x4a3f,  /* B */ 0x00f3,  /* C */
    0x483f,  /* D */ 0x01f3,  /* E */ 0x01c3,  /* F */ 0x02fb,  /* G */
    0x03cc,  /* H */ 0x4833,  /* I */ 0x4863,  /* J */ 0x31c0,  /* K */
    0x00f0,  /* L */ 0x14cc,  /* M */ 0x24cc,  /* N */ 0x00ff,  /* O */
    0x03c7,  /* P */ 0x20ff,  /* Q */ 0x23c7,  /* R */ 0x03bb,  /* S */
    0x4803,  /* T */ 0x00fc,  /* U */ 0x90c0,  /* V */ 0xa0cc,  /* W */
    0xb400,  /* X */ 0x5400,  /* Y */ 0x9033,  /* Z */ 0x00e1,  /* [ */
    0x2400,  /* \ */ 0x001e,  /* ] */ 0xa000,  /* ^ */ 0x0030,  /* _ */
    0x0400,  /* ` */ 0x4170,  /* a */ 0x41e0,  /* b */ 0x0160,  /* c */
    0x4960,  /* d */ 0x8160,  /* e */ 0x4b02,  /* f */ 0x2238,  /* g */
    0x41c0,  /* h */ 0x4000,  /* i */ 0x4020,  /* j */ 0x6a00,  /* k */
    0x4811,  /* l */ 0x4348,  /* m */ 0x4140,  /* n */ 0x4160,  /* o */
    0x09c1,  /* p */ 0x4991,  /* q */ 0x0140,  /* r */ 0x4220,  /* s */
    0x4b10,  /* t */ 0x4060,  /* u */ 0x8040,  /* v */ 0x4078,  /* w */
    0xd800,  /* x */ 0x2038,  /* y */ 0x8120,  /* z */ 0x4912,  /* { */
    0x4800,  /* | */ 0x4a21,  /* } */ 0x0a85,  /* ~ */ 0x0000,  /* DEL */
};

// We need to color segments individually so we can't share vertexes between
// them.
static const vector<vec3> DISPLAY_VERTS = {
    // Segment 0
    vec3( 1, 1, 1 ),    vec3( 2, 2, 1 ),   vec3( 2, 0, 1 ),
    vec3( 2, 2, 1 ),    vec3( 2, 0, 1 ),   vec3( 6, 2, 1 ),
    vec3( 2, 0, 1 ),    vec3( 6, 2, 1 ),   vec3( 6, 0, 1 ),
    vec3( 6, 2, 1 ),    vec3( 6, 0, 1 ),   vec3( 7, 1, 1 ),

    // Segment 1 (Segment 0 + vec(6, 0))
    vec3( 7, 1, 1 ),    vec3( 8, 2, 1 ),    vec3( 8, 0, 1 ),
    vec3( 8, 2, 1 ),    vec3( 8, 0, 1 ),    vec3( 12, 2, 1 ),
    vec3( 8, 0, 1 ),    vec3( 12, 2, 1 ),   vec3( 12, 0, 1 ),
    vec3( 12, 2, 1 ),   vec3( 12, 0, 1 ),   vec3( 13, 1, 1 ),

    // Segment 2
    vec3( 13, 1, 1 ),   vec3( 12, 2, 1 ),   vec3( 14, 2, 1 ),
    vec3( 12, 2, 1 ),   vec3( 14, 2, 1 ),   vec3( 12, 10, 1 ),
    vec3( 14, 2, 1 ),   vec3( 12, 10, 1 ),  vec3( 14, 10, 1 ),
    vec3( 12, 10, 1 ),  vec3( 14, 10, 1 ),  vec3( 13, 11, 1 ),

    // Segment 3 (Segment 2 + vec(0, 10))
    vec3( 13, 11, 1 ),  vec3( 12, 12, 1 ),  vec3( 14, 12, 1 ),
    vec3( 12, 12, 1 ),  vec3( 14, 12, 1 ),  vec3( 12, 20, 1 ),
    vec3( 14, 12, 1 ),  vec3( 12, 20, 1 ),  vec3( 14, 20, 1 ),
    vec3( 12, 20, 1 ),  vec3( 14, 20, 1 ),  vec3( 13, 21, 1 ),

    // Segment 4 (Segment 0 + vec(6, 20))
    vec3( 7, 21, 1 ),   vec3( 8, 22, 1 ),   vec3( 8, 20, 1 ),
    vec3( 8, 22, 1 ),   vec3( 8, 20, 1 ),   vec3( 12, 22, 1 ),
    vec3( 8, 20, 1 ),   vec3( 12, 22, 1 ),  vec3( 12, 20, 1 ),
    vec3( 12, 22, 1 ),  vec3( 12, 20, 1 ),  vec3( 13, 21, 1 ),

    // Segment 5 (Segment 0 + vec(0, 20))
    vec3( 1, 21, 1 ),   vec3( 2, 22, 1 ),   vec3( 2, 20, 1 ),
    vec3( 2, 22, 1 ),   vec3( 2, 20, 1 ),   vec3( 6, 22, 1 ),
    vec3( 2, 20, 1 ),   vec3( 6, 22, 1 ),   vec3( 6, 20, 1 ),
    vec3( 6, 22, 1 ),   vec3( 6, 20, 1 ),   vec3( 7, 21, 1 ),

    // Segment 6 (Segment 2 + vec(-12, -10))
    vec3( 1, 11, 1 ),   vec3( 0, 12, 1 ),   vec3( 2, 12, 1 ),
    vec3( 0, 12, 1 ),   vec3( 2, 12, 1 ),   vec3( 0, 20, 1 ),
    vec3( 2, 12, 1 ),   vec3( 0, 20, 1 ),   vec3( 2, 20, 1 ),
    vec3( 0, 20, 1 ),   vec3( 2, 20, 1 ),   vec3( 1, 21, 1 ),

    // Segment 7 (Segment 2 + vec(-12, 0)
    vec3( 1, 1, 1 ),    vec3( 0, 2, 1 ),    vec3( 2, 2, 1 ),
    vec3( 0, 2, 1 ),    vec3( 2, 2, 1 ),    vec3( 0, 10, 1 ),
    vec3( 2, 2, 1 ),    vec3( 0, 10, 1 ),   vec3( 2, 10, 1 ),
    vec3( 0, 10, 1 ),   vec3( 2, 10, 1 ),   vec3( 1, 11, 1 ),

    // Segment 8 (Segment 0 + vec(0, 10))
    vec3( 1, 11, 1 ),   vec3( 2, 12, 1 ),   vec3( 2, 10, 1 ),
    vec3( 2, 12, 1 ),   vec3( 2, 10, 1 ),   vec3( 6, 12, 1 ),
    vec3( 2, 10, 1 ),   vec3( 6, 12, 1 ),   vec3( 6, 10, 1 ),
    vec3( 6, 12, 1 ),   vec3( 6, 10, 1 ),   vec3( 7, 11, 1 ),

    // Segment 9 (Segment 0 + vec(6, 10))
    vec3( 7, 11, 1 ),   vec3( 8, 12, 1 ),   vec3( 8, 10, 1 ),
    vec3( 8, 12, 1 ),   vec3( 8, 10, 1 ),   vec3( 12, 12, 1 ),
    vec3( 8, 10, 1 ),   vec3( 12, 12, 1 ),  vec3( 12, 10, 1 ),
    vec3( 12, 12, 1 ),  vec3( 12, 10, 1 ),  vec3( 13, 11, 1 ),

    // Segment A
    vec3( 2, 2, 1 ),    vec3( 2, 4.3, 1 ),   vec3( 3, 2, 1 ),
    vec3( 2, 4.3, 1 ),  vec3( 3, 2, 1 ),     vec3( 5.3, 10, 1 ),
    vec3( 3, 2, 1 ),    vec3( 5.3, 10, 1 ),  vec3( 6, 7.2, 1 ),
    vec3( 5.3, 10, 1 ), vec3( 6, 7.2, 1 ),   vec3( 6, 10, 1 ),

    // Segment B (Segment 2 + vec(-6, 0))
    vec3( 7, 1, 1 ),    vec3( 6, 2, 1 ),     vec3( 8, 2, 1 ),
    vec3( 6, 2, 1 ),    vec3( 8, 2, 1 ),     vec3( 6, 10, 1 ),
    vec3( 8, 2, 1 ),    vec3( 6, 10, 1 ),    vec3( 8, 10, 1 ),
    vec3( 6, 10, 1 ),   vec3( 8, 10, 1 ),    vec3( 7, 11, 1 ),

    // Segment C
    vec3( 8, 10, 1 ),   vec3( 8.7, 10, 1 ),  vec3( 8, 7.2, 1 ),
    vec3( 8.7, 10, 1 ), vec3( 8, 7.2, 1 ),   vec3( 12, 4.3, 1 ),
    vec3( 8, 7.2, 1 ),  vec3( 12, 4.3, 1 ),  vec3( 11, 2, 1 ),
    vec3( 12, 4.3, 1 ), vec3( 11, 2, 1 ),    vec3( 12, 2, 1 ),

    // Segment D
    vec3( 8, 12, 1 ),   vec3( 8, 14.8, 1 ),  vec3( 8.7, 12, 1 ),
    vec3( 8, 14.8, 1 ), vec3( 8.7, 12, 1 ),  vec3( 11, 20, 1 ),
    vec3( 8.7, 12, 1 ), vec3( 11, 20, 1 ),   vec3( 12, 17.7, 1 ),
    vec3( 11, 20, 1 ),  vec3( 12, 17.7, 1 ), vec3( 12, 20, 1 ),

    // Segment E (Segment 2 + vec(-6, 10))
    vec3( 7, 11, 1 ),   vec3( 6, 12, 1 ),    vec3( 8, 12, 1 ),
    vec3( 6, 12, 1 ),   vec3( 8, 12, 1 ),    vec3( 6, 20, 1 ),
    vec3( 8, 12, 1 ),   vec3( 6, 20, 1 ),    vec3( 8, 20, 1 ),
    vec3( 6, 20, 1 ),   vec3( 8, 20, 1 ),    vec3( 7, 21, 1 ),

    // Segment F
    vec3( 2, 20, 1 ),   vec3( 3, 20, 1 ),    vec3( 2, 17.7, 1 ),
    vec3( 3, 20, 1 ),   vec3( 2, 17.7, 1 ),  vec3( 6, 14.8, 1 ),
    vec3( 2, 17.7, 1 ), vec3( 6, 14.8, 1 ),  vec3( 5.3, 12, 1 ),
    vec3( 6, 14.8, 1 ), vec3( 5.3, 12, 1 ),  vec3( 6, 12, 1 ),
};
static const unsigned int SEGMENTS = 16;
static const unsigned int VERTS_PER_SEGMENT = 12;

UnionJack::UnionJack(unsigned int length)
    : mLength( length )
{
    mOnColor = vec4( 1, 0, 0, 1 );
    mOffColor = vec4( 0.25, 0, 0, 1 );

    setup();
}

UnionJack& UnionJack::position( const vec2 &pos )
{
    mPosition = pos;
    return *this;
}

UnionJack& UnionJack::below( const UnionJack &other )
{
    mPosition = other.mPosition + vec2( 0, other.height() );
    return *this;
}

UnionJack& UnionJack::rightOf( const UnionJack &other )
{
    mPosition = other.mPosition + vec2( other.width(), other.height() - height() );
    return *this;
}

UnionJack& UnionJack::slant( const float &s )
{
    mSlant = s;
    return *this;
}

UnionJack& UnionJack::scale( const float &s )
{
    mScale = s;
    return *this;
}

UnionJack& UnionJack::colors( const ColorA &on, const ColorA &off )
{
    mOnColor = on;
    mOffColor = off;
    return *this;
}

void UnionJack::setup()
{
    // Group the verts in each segment using the bone index so the shader knows
    // how to color them.
    vector<int> segmentBones;
	for (unsigned int s = 0; s < SEGMENTS; ++s) {
		for (unsigned int v = 0; v < VERTS_PER_SEGMENT; ++v) {
            segmentBones.push_back( s );
        }
    }

    vector<gl::VboMesh::Layout> bufferLayout = {
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::AttribInfo( geom::Attrib::BONE_INDEX, DataType::INTEGER, 1, 0, 0, 0 ) ),
    };
    gl::VboMeshRef mesh = gl::VboMesh::create( DISPLAY_VERTS.size(), GL_TRIANGLES, bufferLayout );
    mesh->bufferAttrib( geom::Attrib::POSITION, DISPLAY_VERTS );
    mesh->bufferAttrib( geom::Attrib::BONE_INDEX, segmentBones );

    std::vector<vec3> characterPosition;
    std::vector<int> segmentValue;

	for (unsigned int d = 0; d < mLength; ++d) {
        characterPosition.push_back( vec3( CHARACTER_WIDTH * d, 0, 0 ) );
        segmentValue.push_back( 0 );
    }

    // TODO: See if we can merge these two VBOs of per instance data. Not clear
    // to me if they have to be of the same data type though.
    mInstanceValueVbo = gl::Vbo::create( GL_ARRAY_BUFFER, segmentValue.size() * sizeof( int ), segmentValue.data(), GL_DYNAMIC_DRAW );
    geom::BufferLayout instanceColorLayout;
    instanceColorLayout.append( geom::Attrib::CUSTOM_0, 1, 0, 0, 1 /* per instance */ );
    mesh->appendVbo( instanceColorLayout, mInstanceValueVbo );

    gl::VboRef instancePositionVbo = gl::Vbo::create( GL_ARRAY_BUFFER, characterPosition.size() * sizeof( vec3 ), characterPosition.data(), GL_STATIC_DRAW );
    geom::BufferLayout instancePositionLayout;
    instancePositionLayout.append( geom::Attrib::CUSTOM_1, 3, 0, 0, 1 /* per instance */ );
    mesh->appendVbo( instancePositionLayout, instancePositionVbo );

    auto shader = gl::GlslProg::create( gl::GlslProg::Format()
        .vertex(    CI_GLSL( 150,
            uniform mat4 ciModelViewProjection;
            uniform vec4 onColor;
            uniform vec4 offColor;

            in vec4 ciPosition;
            // The bone index is used to group the segments.
            in int  ciBoneIndex;
            // Per-instance position of the character, used to shift all vertexes.
            in vec3 vInstancePosition;
            // Per-instance value for the display. Each bit corresponds to the
            // segment's bone index.
            in int  vInstanceValue;

            out lowp vec4 Color;

            void main( void )
            {
                // Discard z to keep us on the xy plane.
                vec4 instanceOffset = vec4( vInstancePosition.xy, 0, 0 );
                gl_Position = ciModelViewProjection * ( ciPosition + instanceOffset );
                // Figure out the vertex color using the display's bits and the bone index.
                Color = (vInstanceValue & (1 << ciBoneIndex)) == 0 ?  offColor: onColor;
            }
         ) )
        .fragment(  CI_GLSL( 150,
            in vec4 Color;

            out vec4 oColor;

            void main( void )
            {
                oColor = Color;
            }
        ) )
    );

    mBatch = gl::Batch::create( mesh, shader, {
        { geom::Attrib::CUSTOM_0, "vInstanceValue" },
        { geom::Attrib::CUSTOM_1, "vInstancePosition" },
    } );
}

// TODO: would be good to store the string and only update the VBO when the
// string changes.
UnionJack& UnionJack::display( string s )
{
    int *value = (int*)mInstanceValueVbo->mapReplace();
	for (unsigned int d = 0, len = s.length(); d < mLength; ++d) {
        // When we get to the end of the input, keep going and blank out the
        // rest of the display.
        *value++ = valueOf( d < len ? s[d] : ' ' );
    }
    mInstanceValueVbo->unmap();

    return *this;
}

uint16_t UnionJack::valueOf( const char input )
{
    // If the input is out of range then just return a blank space.
    if ( input < CHAR_OFFSET || input > CHAR_OFFSET + CHAR_LENGTH ) {
        return charPatterns[0];
    }

    return charPatterns[ input - CHAR_OFFSET ];
}

float UnionJack::height() const
{
    return CHARACTER_HEIGTH * mScale;
}

float UnionJack::width() const
{
    return ( ( CHARACTER_WIDTH * mLength ) + ( CHARACTER_HEIGTH * mSlant ) ) * mScale;
}

Rectf UnionJack::calcBoundingBox() const
{
    return Rectf( mPosition, mPosition + vec2( width(), height() ) );
}

void UnionJack::draw() const
{
    gl::ScopedModelMatrix matrixScope;
    gl::multModelMatrix( modelMatrix() );

    mBatch->getGlslProg()->uniform( "offColor", mOffColor );
    mBatch->getGlslProg()->uniform( "onColor", mOnColor );
    mBatch->drawInstanced( mLength );
}

mat4 UnionJack::modelMatrix() const
{
    mat3 matrix = mat3();
    matrix = translate( matrix, mPosition );
    matrix = glm::scale( matrix, vec2( mScale ) );
    // If there's a slant apply it.
    if ( mSlant != 0.0f ) {
        // Positive slants (to the right) shift the bottom over across our left
        // side so we need to shif everthing over first.
        if ( mSlant > 0.0f ) {
            matrix = translate( matrix, vec2( CHARACTER_HEIGTH * mSlant, 0 ) );
        }
        matrix = shearY( matrix, -mSlant );
    }
    return mat4( matrix );
}
