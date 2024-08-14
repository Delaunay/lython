from dataclasses import dataclass
from typing import Any, Optional
from enum import Enum


class ErrorCode(Enum):
    # -32768 to -32000 are reserved for pre-defined errors
    ParseError     = -32700
    InvalidRequest = -32600
    MethodNotFound = -32601
    InvalidParams  = -32602
    InternalError  = -32603
    # -32000 to -32099


@dataclass
class Request:
    id: Optional[int | str] # if it is not included it is assumed to be a notification
    method: str             # "rpc.(.*)" names a reserved
    params: Optional[Any]
    jsonrpc: str = "2.0"


@dataclass
class Error:
    code: int
    message: str
    data: Optional[Any]


@dataclass
class Response:
    id: int
    result: Optional[Any]
    error: Optional[Error]
    jsonrpc: str = "2.0"



# protobuf
# json
# msgpack