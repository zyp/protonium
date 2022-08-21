import google.protobuf.compiler.plugin_pb2 as plugin_pb2
import google.protobuf.descriptor_pb2 as descriptor_pb2
import io
import sys

from . import options_pb2

import jinja2

#loader = jinja2.FileSystemLoader('.')

jinja2_env = jinja2.Environment(
    #loader = loader,
    loader = jinja2.PackageLoader('protonium'),
    trim_blocks = True,
    lstrip_blocks = True,
)

jinja2_template = jinja2_env.get_template('generator_template.h.j2')

class OneOf:
    def __init__(self, data):
        self.name = data.name
        self.fields = []
    
    def add_field(self, field):
        self.fields.append(field)
        return self.name, len(self.fields) - 1

    @property
    def cpp_type(self):
        types = ', '.join(field.cpp_type_base for field in self.fields)
        return f'oneof<{types}>'

class Field:
    def __init__(self, data, message):
        fd = descriptor_pb2.FieldDescriptorProto

        self.data = data
        self.name = data.name
        self.number = data.number
        self.type = data.type
        self.type_name = data.type_name
        self.repeated = data.label == fd.LABEL_REPEATED
        self.packed = data.options.packed

        #print(data, file = sys.stderr)

        if data.HasField('oneof_index'):
            self.oneof, self.oneof_idx = message.oneofs[data.oneof_index].add_field(self)
            self.initializer = f'{{{self.oneof}}}'
        else:
            self.oneof = None

    @property
    def cpp_type_base(self):
        fd = descriptor_pb2.FieldDescriptorProto

        type_name = self.type_name.split('.')[-1]

        if self.type == fd.TYPE_MESSAGE:
            return type_name

        if self.type == fd.TYPE_ENUM:
            return f'enum_<{ type_name }>'

        return {
            fd.TYPE_INT32: 'int32',
            fd.TYPE_INT64: 'int64',
            fd.TYPE_UINT32: 'uint32',
            fd.TYPE_UINT64: 'uint64',
            fd.TYPE_SINT32: 'sint32',
            fd.TYPE_SINT64: 'sint64',
            fd.TYPE_FIXED32: 'fixed32',
            fd.TYPE_FIXED64: 'fixed64',
            fd.TYPE_SFIXED32: 'sfixed32',
            fd.TYPE_SFIXED64: 'sfixed64',
            fd.TYPE_FLOAT: 'float_',
            fd.TYPE_DOUBLE: 'double_',
            fd.TYPE_BOOL: 'bool_',
            fd.TYPE_STRING: 'string<storage_class::dynamic>',
            fd.TYPE_BYTES: 'bytes<storage_class::dynamic>',
            #fd.TYPE_STRING: 'string<storage_class::static_<16>>',
            #fd.TYPE_BYTES: 'bytes<storage_class::static_<16>>',
        }[self.type];

    @property
    def cpp_type(self):
        if self.oneof:
            return f'oneof_field<{ self.oneof_idx }, decltype({ self.oneof })>'

        if self.repeated:
            return f'repeated<{ self.cpp_type_base }, storage_class::dynamic, { str(bool(self.packed)).lower() }>'
            #return f'repeated<{ self.cpp_type_base }, storage_class::static_<16>, { str(bool(self.packed)).lower() }>'

        return self.cpp_type_base

    @property
    def tag_type(self):
        fd = descriptor_pb2.FieldDescriptorProto

        if self.repeated and self.packed:
            return 'length_delimited'

        return {
            fd.TYPE_MESSAGE: 'length_delimited',
            fd.TYPE_INT32: 'varint',
            fd.TYPE_INT64: 'varint',
            fd.TYPE_UINT32: 'varint',
            fd.TYPE_UINT64: 'varint',
            fd.TYPE_SINT32: 'varint',
            fd.TYPE_SINT64: 'varint',
            fd.TYPE_FIXED32: 'fixed32',
            fd.TYPE_FIXED64: 'fixed64',
            fd.TYPE_SFIXED32: 'fixed32',
            fd.TYPE_SFIXED64: 'fixed64',
            fd.TYPE_FLOAT: 'fixed32',
            fd.TYPE_DOUBLE: 'fixed64',
            fd.TYPE_BOOL: 'varint',
            fd.TYPE_ENUM: 'varint',
            fd.TYPE_STRING: 'length_delimited',
            fd.TYPE_BYTES: 'length_delimited',
        }[self.type];

    @property
    def tag(self):
        return f'tag_t {{{self.number}, tag_t::{self.tag_type}}}'

class Enum:
    def __init__(self, data):
        self.name = data.name
        self.entries = sorted((e.number, e.name) for e in data.value)

class Message:
    def __init__(self, data):
        self.name = data.name
        self.oneofs = [OneOf(f) for f in data.oneof_decl]
        self.fields = [Field(f, self) for f in data.field]
        self.nested_messages = [Message(m) for m in data.nested_type]
        self.enums = [Enum(e) for e in data.enum_type]

class Method:
    def __init__(self, data):
        self.name = data.name
        self.number = data.options.Extensions[options_pb2.protonium_method].number
        self.request_type = data.input_type.split('.')[-1]
        self.response_type = data.output_type.split('.')[-1]

class Service:
    def __init__(self, data):
        self.name = data.name
        self.number = data.options.Extensions[options_pb2.protonium_service].number
        self.methods = [Method(m) for m in data.method]

class File:
    def __init__(self, data):
        self.messages = [Message(m) for m in data.message_type]
        self.services = [Service(s) for s in data.service]
        self.enums = [Enum(e) for e in data.enum_type]
        self.ignore = data.options.Extensions[options_pb2.protonium_file].ignore
        #print('options', data.name, data.options.Extensions[options_pb2.protonium_file], file = sys.stderr)
        self.cpp_includes = [filename_translate(filename) for filename in data.dependency if not context_files[filename].ignore]
        #print('includes', self.cpp_includes, file = sys.stderr)

        #print(data.name, data.package, file = sys.stderr)


def read_stdin():
    with io.open(sys.stdin.fileno(), 'rb') as f:
        return f.read()

def write_stdout(data):
    with io.open(sys.stdout.fileno(), 'wb') as f:
        f.write(data)

def read_request():
    data = read_stdin()
    return plugin_pb2.CodeGeneratorRequest.FromString(data)

def filename_translate(filename):
    assert filename.endswith('.proto')
    return filename.removesuffix('.proto') + '_pb.h'

def filename_translate_python(filename):
    assert filename.endswith('.proto')
    return filename.removesuffix('.proto') + '_pb2.py'

context_files = {}

def generate():
    request = read_request()
    response = plugin_pb2.CodeGeneratorResponse()

    for proto_file in request.proto_file:
        #print(proto_file.name, list(proto_file.dependency), file = sys.stderr)

        context_files[proto_file.name] = file = File(proto_file)

        if proto_file.name not in request.file_to_generate:
            continue

        output_name = filename_translate(proto_file.name)
        output_data = jinja2_template.render(file = file)

        response.file.add(name = output_name, content = output_data)

    write_stdout(response.SerializeToString())

def generate_python():
    request = read_request()
    response = plugin_pb2.CodeGeneratorResponse()

    for proto_file in request.proto_file:
        if proto_file.name not in request.file_to_generate:
            continue

        output_name = filename_translate_python(proto_file.name)
        output_data = ''

        for service in proto_file.service:
            output_data += f'class {service.name}(ServiceBase):\n'
            output_data += f'    service_number = {service.options.Extensions[options_pb2.protonium_service].number}\n\n'

            for method in service.method:
                output_data += f'    {method.name} = RPCMethod({method.options.Extensions[options_pb2.protonium_method].number}, \'{method.input_type[1:]}\', \'{method.output_type[1:]}\')\n'
            
            output_data += '\n'

        #print(output_name, file = sys.stderr)

        response.file.add(name = output_name, insertion_point = 'module_scope', content = output_data)

        response.file.add(name = output_name, insertion_point = 'imports', content = 'from protonium.python import ServiceBase, RPCMethod\n')

    write_stdout(response.SerializeToString())
