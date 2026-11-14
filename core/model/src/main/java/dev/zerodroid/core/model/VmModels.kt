package dev.zerodroid.core.model
enum class VmState { CREATED,PREFLIGHT,PREPARED,STARTING,RUNNING,STOPPING,STOPPED,FAILED,DESTROYING,DESTROYED }
data class VmId(val value:String) { init { require(value.isNotBlank() && value.length<=128) } }
data class GuestProfileRef(val profileId:String,val revision:String)
data class ResourceRange(val minimum:Long,val maximum:Long,val adaptive:Boolean) { init { require(minimum>=0L); require(maximum>=minimum) } }
data class VmResourcePolicy(val vcpus:ResourceRange,val memoryBytes:ResourceRange,val storageQuotaBytes:Long) { init { require(storageQuotaBytes>0L) } }
data class VmSpec(val id:VmId,val displayName:String,val guestProfile:GuestProfileRef,val imageSetId:String,val resources:VmResourcePolicy) {
 init { require(displayName.isNotBlank()); require(imageSetId.isNotBlank()) }
}
