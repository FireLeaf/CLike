package.path = package.path .. ";./lua/protobuf/?.lua;./lua/pb/?.lua;"
require "hello_pb"
local hello_pb_ = hello_pb.hello()
function main()
	hello_pb_.age = 25
	hello_pb_.name = "yangcao"
	hello_pb_.is_male = true
	hello_pb_.hobbies:append(hello_pb.hello.football) --= {hello_pb.football, hello_pb.swimming}
	hello_pb_.hobbies:append(hello_pb.hello.swimming)
	local data = hello_pb_:SerializeToString()
	local msg = hello_pb.hello()
	msg:ParseFromString(data)
	print(msg)
end
main()
