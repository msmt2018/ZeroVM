package dev.zerodroid.core.common
sealed interface ZeroDroidResult<out T> {
 data class Success<T>(val value:T):ZeroDroidResult<T>
 data class Failure(val error:ZeroDroidError):ZeroDroidResult<Nothing>
}
data class ZeroDroidError(val domain:ErrorDomain,val code:String,val message:String,val retryable:Boolean=false,val operationId:String?=null)
enum class ErrorDomain { VALIDATION,IMAGE,PROFILE,HOST,PERMISSION,BACKEND,BOOT,DEVICE,BRIDGE,STORAGE,SECURITY,RESOURCE,INTERNAL }
