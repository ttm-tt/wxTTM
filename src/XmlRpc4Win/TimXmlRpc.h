/*
XmlRpc C++ client for Windows
-----------------------------

Created by Dr Tim Cooper,  tco@smartsgroup.com,   March 2009.

This lets you talk to web sites using XmlRpc from C++ on Windows.  It differs
from similar libraries as follows:
	- works on Windows
	- supports HTTPS (SSL)
	- uses wininet to manage HTTP/HTTPS, so it's really very minimal
	- much faster than XmlRpc++ which suffers from STL performance problems.

This project consists of 2 files:   "TimXmlRpc.h"  and  "TimXmlRpc.cpp".

Parts of this project have been taken from Chris Morley's "XmlRpc++" project,
in particular the API.  Chris's contribution is acknowledged by marking his
work with the token:  "/*ChrisMorley/".  Thanks, Chris!


-------------------------Sample app:-------------------------

#include <iostream>
#include "timXmlRpc.h"

void Test(std::string username, std::string password)
{
	XmlRpcValue args, result;

	XmlRpcClient Connection("https://www.edval.com.au:9001/test.php");
	args[0] = username;
	args[1] = Md5(username + "." + password);		// salted Md5
	if (not Connection.execute("tetun.aprende", args, result)) {
		std::cerr << "Error: " << Connection.getError() << std::endl;
	}
	else {
		std::cout << "The answer is: " << std::string(result) << std::endl;
	}
}


See 'SampleMain.cpp' for a more elaborate example.

*/



#include <string>
#include <vector>
#include <map>
#include <sstream>




typedef void (*XmlRpcCallback)(void* context, const char* status);


/* A 'get username and password' function is used for HTTP basic authentication.
It will either get the (username,password) pair from some stored location, or
prompt the user for it. 
	Return 'true' if the user attempted to supply the credentials, or 'false'
if they want to cancel.
	Display a 'logon failed' message if 'retry'.
*/
typedef bool (*getBasicAuth_UsernameAndPassword_fn)(bool retry, char username[256], char password[256]);


	/* <Chris Morley> */
class XmlRpcException {
	std::string _message;

public:
	//! Constructor
	//!	 @param message	A descriptive error message
	XmlRpcException(const std::string message) :
			_message(message) {}

	//! Return the error message.
	const std::string& getMessage() const { return _message; }
};
	/* </Chris Morley> */


class XmlRpcValue {
public:
	/* <Chris Morley> */
	enum class Type {
		TypeInvalid,
		TypeBoolean,
		TypeInt,
		TypeDouble,
		TypeString,
		TypeDateTime,
		TypeBase64,
		TypeArray,
		TypeStruct,
		TypeNil
	};

	// Non-primitive types
	typedef std::vector<char> BinaryData;
	typedef std::map<std::string, XmlRpcValue> ValueStruct;
	/* </Chris Morley> */

	class ValueArray {
		// tco> I'm implementing my own 'ValueArray' instead of the original
		// std::vector<> because resizing the std::vector<> calls 100's of 
		// constructors and destructors.  Using 'vector::reserve()' is not sufficient
		// to prevent these constructors/destructors from being called, because
		// the C++ standard requires constructors and destructors be called whenever
		// an object changes its address, as happens when the std::vector resizes.
		XmlRpcValue *A;
		int _size;
		int _allocated;

		ValueArray& operator=(const ValueArray &other);

	public:
		ValueArray() { A = NULL; _size = _allocated = 0; }
		ValueArray(int n) { 
					A = NULL; _size = _allocated = 0;
					resize(n); }
		ValueArray(ValueArray &other);
		int size() { return _size; }
		void resize(int n);
		XmlRpcValue& operator[](int i) { return A[i]; }
		XmlRpcValue& at(int i) { return A[i]; }
		bool operator==(ValueArray &other);
		void push_back(XmlRpcValue &val) { int last = _size; resize(_size + 1); A[last] = val; }
		~ValueArray();
	};

	/* <Chris Morley> */

	//! Constructors
	XmlRpcValue() : _type(Type::TypeInvalid) { u.asBinary = 0; }
	XmlRpcValue(bool value) : _type(Type::TypeBoolean) { u.asBool = value; }
	XmlRpcValue(int value)	: _type(Type::TypeInt) { u.asInt = value; }
	XmlRpcValue(double value)	: _type(Type::TypeDouble) { u.asDouble = value; }
	XmlRpcValue(char value)	: _type(Type::TypeString) { 
					u.asString = (char*)malloc(2); 
					u.asString[0] = value;
					u.asString[1] = '\0';
				}

	XmlRpcValue(std::string const& value) : _type(Type::TypeString) 
	{ u.asString = _strdup(value.c_str()); }

	XmlRpcValue(const char* value)	: _type(Type::TypeString)
	{ u.asString = _strdup(value); }

	XmlRpcValue(struct tm* value)	: _type(Type::TypeDateTime) 
	{ u.asTime = new struct tm(*value); }

	XmlRpcValue(void* value, int nBytes)	: _type(Type::TypeBase64)
	{
		u.asBinary = new BinaryData((char*)value, ((char*)value)+nBytes);
	}

	//! Copy
	XmlRpcValue(XmlRpcValue const& rhs) : _type(Type::TypeInvalid) { *this = rhs; }

	//! Destructor (make virtual if you want to subclass)
	/*virtual*/ ~XmlRpcValue() { invalidate(); }

	//! Erase the current value
	void clear() { invalidate(); }

	// Operators
	XmlRpcValue& operator=(bool rhs) { return operator=(XmlRpcValue(rhs)); }
	XmlRpcValue& operator=(int const& rhs) { return operator=(XmlRpcValue(rhs)); }
	XmlRpcValue& operator=(double const& rhs) { return operator=(XmlRpcValue(rhs)); }
	XmlRpcValue& operator=(const char* rhs) { return operator=(XmlRpcValue(rhs)); }
	XmlRpcValue& operator=(char rhs) { return operator=(XmlRpcValue(rhs)); }

	XmlRpcValue& operator=(XmlRpcValue const& rhs);	//<-- don't use copy constructors if you can avoid them!
				// This does a deep copy. Often you can use references instead of using copy constructors.

	bool operator==(XmlRpcValue const& other) const;
	bool operator!=(XmlRpcValue const& other) const { return !(*this == other); }

	std::string GetStdString()	{
								if (_type == Type::TypeInt) {
									char tmp[16] = {0};
									_itoa_s(u.asInt, tmp, 10);
									return tmp;
								}
								assertTypeOrInvalid(Type::TypeString);
								return u.asString;
							}

	// There are some basic type conversions here. This might mean that your
	// program parses stuff that strictly speaking it should report as a type error.

	operator bool()			{	assertTypeOrInvalid(Type::TypeBoolean); 
								return u.asBool; 
							}
	operator int()			{								
								if (_type == Type::TypeString && u.asString[0] >= '0' && u.asString[0] <= '9')
									return atoi(u.asString);
								if (_type == Type::TypeDouble)
									return (int)u.asDouble;
								if (_type == Type::TypeInt)
									return u.asInt;
								assertTypeOrInvalid(Type::TypeInt);
								return 0;
							}
	operator char()			{ assertTypeOrInvalid(Type::TypeString); return *u.asString; }
	operator double()		{	if (_type == Type::TypeDouble)
									return u.asDouble; 
								if (_type == Type::TypeInt)
									return u.asInt;
								assertTypeOrInvalid(Type::TypeDouble);
								return 0;
							}

	operator const char*()	{ assertTypeOrInvalid(Type::TypeString); return u.asString; }
	operator BinaryData&()	{ assertTypeOrInvalid(Type::TypeBase64); return *u.asBinary; }
	operator struct tm&()	{ assertTypeOrInvalid(Type::TypeDateTime); return *u.asTime; }
	operator ValueStruct&()	{ assertTypeOrInvalid(Type::TypeStruct); return *u.asStruct; }	// good for iterating thru fields

	XmlRpcValue const& operator[](int i) const { 
			assertArray(i+1); return u.asArray->at(i); 
	}
	XmlRpcValue& operator[](int i)	{ assertArray(i+1); return u.asArray->at(i); }

	XmlRpcValue& operator[](std::string const& k) { assertStruct(); return (*u.asStruct)[k]; }
	XmlRpcValue& operator[](const char* k) { assertStruct(); std::string s(k); return (*u.asStruct)[s]; }

	// Accessors
	//! Return true if the value has been set to something.
	bool valid() const { return _type != Type::TypeInvalid; }

	//! Return the type of the value stored. \see Type.
	Type const &getType() const { return _type; }

	//! Return the size for string, base64, array, and struct values.
	int size() const;

	//! Set up this value as an array, if not already so.  This function is optional,
	// because an undefined value is converted to an array implicitly the first time
	// you index it with an integer e.g. arg[0] = "hello";   , however if there's a
	// chance your array will be zero length then this function is compulsory.
	void initAsArray() { assertArray(0); }

	//! Specify the size for array values. Array values will grow beyond this size if needed.
	void setSize(int size)		{ assertArray(size); }

	//! Check for the existence of a struct member by name.
	bool hasMember(const std::string& name) const;

	//! Decode xml. Destroys any existing value.
	void fromXml(const char* &s);

	//! Encode the Value in xml
	void toXml(std::ostringstream &ostr) const;

	bool parseMethodResponse(const char* s);
	void buildCall(const char* method, std::ostringstream &ostr) const;

protected:
	// Clean up
	void invalidate();

	// Type checking
	void assertTypeOrInvalid(Type t);
	void assertArray(int size) const;
	void assertArray(int size);
	void assertStruct();

	// XML decoding
	void boolFromXml(const char* &s);
	void intFromXml(const char* &s);
	void doubleFromXml(const char* &s);
	void stringFromXml(const char* &s);
	void timeFromXml(const char* &s);
	void binaryFromXml(const char* &s);
	void arrayFromXml(const char* &s);
	void structFromXml(const char* &s);

	// XML encoding
	void boolToXml(std::ostringstream &ostr) const;
	void intToXml(std::ostringstream &ostr) const;
	void doubleToXml(std::ostringstream &ostr) const;
	void stringToXml(std::ostringstream &ostr) const;
	void timeToXml(std::ostringstream &ostr) const;
	void binaryToXml(std::ostringstream &ostr) const;
	void arrayToXml(std::ostringstream &ostr) const;
	void structToXml(std::ostringstream &ostr) const;
	void nilToXml(std::ostringstream &ostr) const;

	// Type tag and values
	Type _type;

	union {
		bool			asBool;
		int				asInt;
		double			asDouble;
		struct tm*		asTime;
		char*			asString;
		BinaryData*		asBinary;
		ValueArray*		asArray;
		ValueStruct*	asStruct;
	} u;
	
};
	/* </Chris Morley> */


class XmlRpcClient {
	class XmlRpcImplementation *secret;

public:
	enum protocol_enum { XMLRPC_AUTO=0, XMLRPC_HTTP=1, XMLRPC_HTTPS=2 };

  //! Default constructor
  XmlRpcClient() : secret(nullptr) {}

	//! Construct a client and attempt to connect to the server at the specified host:port address
	//!	@param server The name of the remote machine hosting the server
	//!	@param port The port on the remote machine where the server is listening
	//!	@param object	An optional object name to be sent in the HTTP GET header
	XmlRpcClient(const char* server, int port, const char* object, protocol_enum protocol=XMLRPC_AUTO);

	//! Construct a client and attempt to connect to the server at the specified URI.
	//!	@param URI  (Commonly and previously known as "URL"): e.g. "https://www.edval.com.au:9001/test.php"
	XmlRpcClient(const char* URI);

	~XmlRpcClient() { close(); }

  //! Connect to the server
	//!	@param URI The URI to connect to
	//!	@param user The user name to use for authentication
	//!	@param password	The password to use for authentication
	bool connect(const char *URI, const char *user = nullptr, const char *password = nullptr);

	//! Execute the named procedure on the remote server.
	//!	@param method The name of the remote procedure to execute
	//!	@param params An array of the arguments for the method
	//!	@param result The result value to be returned to the client
	//!	@return true if the request was sent and a result received 
	//!	 (although the result might be a fault).
	//!
	//! Currently this is a synchronous (blocking) implementation (execute
	//! does not return until it receives a response or an error). Use isFault()
	//! to determine whether the result is a fault response.
	bool execute(const char* method, XmlRpcValue const& params, XmlRpcValue& result);

	//! Returns true if the result of the last execute() was a fault response.
	bool isFault() const;

	// Set the details for a callback function
	void setCallback(XmlRpcCallback Callback, void* context);

	// Set a callback to pop up a dialog box to ask the user for credentials
	void setBasicAuth_Callback(getBasicAuth_UsernameAndPassword_fn fn);

	// If you already have the credentials, pass them in here.
	void setBasicAuth_UsernameAndPassword(const char* username, const char* password);

	// ignore the certificate authority on subsequent execute()'s.
	void setIgnoreCertificateAuthority(bool value=true);

	// Get and set error messages:
	std::string getError();
	void setError(std::string);
	int getHttpErrorCode();

	//! Close the connection
	void close();
};
