package com.jkk.exception;

public enum  ErrorCode {
    //成功
    EASYLITE_SUCCESS(0),
    //jni类找不到
    EASYLITE_ERR_JNI_CLASS_NOTFOUND(20),
    ;
    private int code;
    private ErrorCode(int code){
        this.code=code;
    }
    public int getCode(){
        return code;
    }
}

