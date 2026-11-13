package dev.zerovm.core.runtime
import dev.zerovm.core.common.ZeroVmResult
import dev.zerovm.core.model.VmId
import dev.zerovm.core.model.VmSpec
import dev.zerovm.core.model.VmState
import kotlinx.coroutines.flow.Flow
data class VmSnapshot(val id:VmId,val state:VmState,val stateRevision:Long,val backendId:String?)
data class VmEvent(val vmId:VmId,val sequence:Long,val type:String,val timestampEpochMillis:Long,val operationId:String?)
data class VmOperation(val id:String,val vmId:VmId,val kind:Kind,val acceptedAtEpochMillis:Long) { enum class Kind { CREATE,START,SHUTDOWN,STOP,DESTROY } }
interface ZeroVmRuntime:AutoCloseable {
 suspend fun create(spec:VmSpec):ZeroVmResult<VmOperation>
 suspend fun start(id:VmId):ZeroVmResult<VmOperation>
 suspend fun requestShutdown(id:VmId):ZeroVmResult<VmOperation>
 suspend fun stop(id:VmId):ZeroVmResult<VmOperation>
 suspend fun destroy(id:VmId):ZeroVmResult<Unit>
 suspend fun snapshot(id:VmId):ZeroVmResult<VmSnapshot>
 fun observeEvents(id:VmId):Flow<VmEvent>
 override fun close()
}
