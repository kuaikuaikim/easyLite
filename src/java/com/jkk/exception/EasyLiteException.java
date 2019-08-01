package com.jkk.exception;

/**
 * @brief 数据库异常类
 */
public class EasyLiteException extends RuntimeException{
    private static final long serialVersionUID = 1L;
    /**
     * @brief 异常错误码
     */
    private int code;
    public EasyLiteException(){
        super();
    }
    public EasyLiteException(String msg, int errorCode){
        super(msg);
        code=errorCode;
    }
    public int getCode() {
        return code;
    }
}
