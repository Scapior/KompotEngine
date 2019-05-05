#pragma once

enum class IOError
{
    Successfull,
    Unknown,
    FileOrDirectoryNotFound,
    FileAlreadyExists,
    AccessDenied,
    UnexpectedEndOfFile,
    WrongKemFileFormat
};
