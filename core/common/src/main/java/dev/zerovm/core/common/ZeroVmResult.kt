package dev.zerovm.core.common
sealed interface ZeroVmResult<out T> {
 data class Success<T>(val value:T):ZeroVmResult<T>
 data class Failure(val error:ZeroVmError):ZeroVmResult<Nothing>
}
data class ZeroVmError(val domain:ErrorDomain,val code:String,val message:String,val retryable:Boolean=false,val operationId:String?=null)
enum class ErrorDomain { VALIDATION,IMAGE,PROFILE,HOST,PERMISSION,BACKEND,BOOT,DEVICE,BRIDGE,STORAGE,SECURITY,RESOURCE,INTERNAL }
