# CLAUDE.md - Stream Module

## Module Overview

The Stream module provides a comprehensive binary serialization and deserialization framework for C++ with support for multiple stream backends (memory, file, network). It offers type-safe streaming operations with version control, endianness handling, and integration with Malterlib's container types. The module is designed for high performance with minimal allocations and supports both intrusive and non-intrusive streaming patterns.

## Key Components

### Core Stream Classes
- **CBinaryStream** - Abstract base class for all binary streams
	- Provides fundamental feed/consume operations
	- Version management support
	- Direction control (feed vs consume)
	- Operator overloading for intuitive << >> % syntax

- **CBinaryStreamDefault** - Default implementation with common operations
	- Base for most concrete stream implementations
	- Provides standard serialization for basic types
	- Container streaming support with length limits

- **CBinaryStreamMemory** - In-memory stream implementation
	- Template-based for different vector types (CByteVector, CSecureByteVector)
	- Dynamic buffer management with growth policies
	- Zero-copy operations via move semantics
	- Direct buffer access for performance

- **CBinaryStreamMemoryPtr** - Memory pointer-based stream
	- Read-only or read-write access to existing buffers
	- No allocation overhead
	- Ideal for deserializing existing data

### Stream Utilities
- **CScopeBinaryStreamVersion** - RAII version management
	- Temporarily sets stream version
	- Automatically restores previous version on destruction
	- Ensures version consistency in nested operations

- **Stream Wrappers** - Type-safe stream adapters
	- `fg_GetUnsafeStreamWrapper()` - Bypasses type safety for raw operations
	- Reference and pointer stream wrappers
	- Custom type streaming support

### ByteVector Integration
- **ByteVector Conversion Functions**
	- `fg_ToByteVector()` / `fg_FromByteVector()` - Standard endianness
	- `fg_ToByteVectorBE()` / `fg_FromByteVectorBE()` - Big-endian variants
	- Secure variants for sensitive data
	- Template-based for any streamable type

### Stream Types Support
Specialized streaming for:
- **Containers** - Vector, LinkedList, Map, Set, Registry
	- BitArray with hierarchical variants
	- Regions for memory management
- **Intrusive Types** - AVL trees, linked lists
- **Time Types** - Time and TimeSpan
- **String Types** - CStr with encoding support
- **Float/Int Types** - With endianness control

## Module-Specific Conventions

### Namespace Organization
- Primary namespace: `NMib::NStream`
- Exception types in main namespace
- Implementation details in source files

### Naming Patterns
- Stream classes: `CBinaryStream[Type]` (e.g., `CBinaryStreamMemory`)
- Stream types: `TC[Type]Stream` for templates
- Conversion functions: `fg_To[Type]()` / `fg_From[Type]()`
- Wrapper functions: `fg_Get[Type]StreamWrapper()`

### Operator Conventions
```cpp
// Feed (serialize) operations
Stream << Value;				// Feed single value
Stream << Ptr;					// Feed via pointer

// Consume (deserialize) operations
Stream >> Value;				// Consume into value
Stream >> Ptr;					// Consume via pointer

// Bidirectional operations
Stream % Value;					// Stream based on current direction
```

### Macro Usage
- `DMibStreamImplementOperators` - Adds operator overloads
- `DMibStreamImplementProtected` - Adds protected implementation
- `DMibErrorStream` - Stream-specific error handling
- `DMibErrorStreamVersionMismatch` - Version mismatch errors

## Dependencies

### Internal Malterlib Modules
- **Core** - Basic types, exceptions, platform abstractions
- **Container** - Vector types for storage
- **Memory** - Memory operations (memcpy, etc.)
- **File** - File I/O for file-based streams
- **String** - String serialization
- **Encoding** - Character encoding support

## Architecture Details

### Stream Direction Model
```cpp
enum EStreamDirection
{
	EStreamDirection_Feed,			// Writing/Serializing
	EStreamDirection_Consume		// Reading/Deserializing
};
```

### Version Management
```cpp
// Scoped version setting
CScopeBinaryStreamVersion Version(Stream, 2);
Stream << VersionedData;			// Uses version 2
// Version automatically restored here
```

### Memory Stream Architecture
```cpp
template <typename t_CStreamType = CBinaryStreamDefault,
		  typename t_CVector = CByteVector>
class CBinaryStreamMemory : public t_CStreamType
{
	umint m_Position;				// Current position
	umint m_Length;					// Valid data length
	umint m_BufferSize;				// Allocated size
	uint8 *m_pBuffer;				// Direct buffer pointer
	t_CVector m_Buffer;				// Underlying storage
};
```

### Stream Type Safety
```cpp
// Type-safe streaming with concepts
template <typename t_CType>
concept cIsValidStreamVersion =
	(NTraits::cIsEnum<t_CType> && sizeof(t_CType) <= 4 && !NTraits::cIsSigned<...>)
	|| (NTraits::cIsInteger<t_CType> && sizeof(t_CType) <= 4 && !NTraits::cIsSigned<...>);
```

## Common Tasks

### Basic Serialization
```cpp
// Serialize to memory
CBinaryStreamMemory<> OutStream;
OutStream << uint32(42);
OutStream << "Hello World";
OutStream << MyObject;

// Get serialized data
auto Data = OutStream.f_MoveVector();
```

### Deserialization
```cpp
// Deserialize from buffer
CBinaryStreamMemoryPtr<> InStream;
InStream.f_OpenRead(Data.f_GetArray(), Data.f_GetLen());

uint32 Value;
CStr Text;
CMyObject Object;
InStream >> Value >> Text >> Object;
```

### Version-Aware Streaming
```cpp
void CMyClass::f_Stream(CBinaryStream &_Stream)
{
	uint32 Version = 3;
	_Stream % Version;

	CScopeBinaryStreamVersion ScopeVersion(_Stream, Version);

	_Stream % m_BasicData;

	if (Version >= 2)
		_Stream % m_ExtendedData;

	if (Version >= 3)
		_Stream % m_NewFeature;
}
```

### ByteVector Conversions
```cpp
// Convert object to byte vector
CMyData Data;
auto Bytes = fg_ToByteVector(Data);

// Reconstruct from bytes
auto RestoredData = fg_FromByteVector<CMyData>(Bytes);

// Big-endian for network
auto NetworkBytes = fg_ToByteVectorBE(Data);
```

### Custom Type Streaming
```cpp
class CMyType
{
public:
	void f_Feed(CBinaryStream &_Stream) const
	{
		_Stream << m_Value1 << m_Value2;
	}

	void f_Consume(CBinaryStream &_Stream)
	{
		_Stream >> m_Value1 >> m_Value2;
	}

	void f_Stream(CBinaryStream &_Stream)
	{
		_Stream % m_Value1 % m_Value2;
	}

private:
	int m_Value1;
	CStr m_Value2;
};
```

### Container Streaming with Length Limits
```cpp
// Set length limit to prevent DOS attacks
Stream.f_SetContainerLengthLimit(1024 * 1024);	// 1MB max

CVector<int> LargeVector;
Stream >> LargeVector;							// Throws if too large
```

### Running Module Tests
```bash
# Build tests
MalterlibBuildShowProgress=false ./mib build Tests

# Run all stream tests
/opt/Deploy/Tests/RunAllTests --paths '["Malterlib/Stream/*"]'

# Run specific test suites
/opt/Deploy/Tests/RunAllTests --paths '["Malterlib/Stream/Wrapper", "Malterlib/Stream/Length Limit"]'
```

## Important Files

### Headers (Public API)
- `Include/Mib/Stream/Binary` - Main binary stream header
- `Include/Mib/Stream/ByteVector` - ByteVector conversion utilities
- `Include/Mib/Stream/Streams/*` - Specialized stream implementations
- `Include/Mib/Stream/Types/*` - Type-specific streaming

### Core Implementation
- `Source/Malterlib_Stream.h/cpp` - Core stream classes
- `Source/Malterlib_Stream.hpp` - Template implementations
- `Source/Malterlib_Stream_Memory.h` - Memory stream implementation
- `Source/Malterlib_Stream_ByteVector.h/hpp` - ByteVector conversions

### Specialized Implementations
- `Source/Malterlib_Stream_Container_LinkedList.h` - LinkedList streaming
- `Source/Malterlib_Stream_Indirection.h` - Indirection/wrapper support

### Tests
- `Test/Test_Malterlib_Stream.cpp` - Core stream tests

## Module-Specific Notes

### Performance Characteristics
- **Memory Streams**: O(1) read/write at current position
- **Buffer Growth**: Geometric growth (typically 2x) to amortize allocations
- **Move Operations**: Zero-copy via move semantics
- **Direct Buffer Access**: Available for performance-critical code

### Endianness Handling
- Default functions use little endian
- BE variants for network/cross-platform compatibility
- Automatic conversion in stream operators

### Memory Management
- Memory streams manage their own buffers
- Pointer streams use external buffers (no allocation)
- Move semantics prevent unnecessary copies
- Secure vectors for sensitive data (automatic zeroing)

### Thread Safety
- Streams are NOT thread-safe
- Each thread should use its own stream instance
- Shared data requires external synchronization

### Error Handling
- `CExceptionStream` - General stream errors
- `CExceptionStreamVersionMismatch` - Version incompatibility
- End-of-stream detection via exceptions
- Length limit violations throw immediately

### Stream Position Management
- Position tracking in umint (platform-specific integer)
- Supports seek operations (absolute, relative, from-end)
- Position validation for read operations
- Automatic position updates during streaming

### Version Control Best Practices
- Always version your streamable classes
- Use CScopeBinaryStreamVersion for automatic management
- Increment version when adding fields
- Use enums to name your versions
- Maintain backward compatibility when possible
- Document version changes in class comments

### Integration with Other Modules
- Seamless Container module integration
- Time module types have built-in streaming
- String module encoding support
- Cryptography module for secure streams

### Design Patterns
- RAII for version management
- Template-based for type flexibility
- Operator overloading for intuitive syntax
- Concept constraints for compile-time safety

### Known Limitations
- Memory streams limited to umint size (platform-dependent)
- No built-in compression (use Compression module)
- No automatic encryption (use Cryptography module)
- Version must fit in uint32

### Best Practices
- Use memory streams for small data
- Consider file streams for large data
- Always set container length limits in untrusted scenarios, but by default length limits are set to the stream length if available
- Prefer move operations for large buffers
- Use secure vectors for passwords/keys
- Implement only f_Stream as it's easier to maintain and read. f_Feed and f_Consume only when there are complex differences.
- Test serialization round-trips in unit tests
