<div class="info">
	<div class="panel panel-default">
		<div class="panel-heading">
			<h3 class="panel-title">[[admin/development/info:you-are-on, {host}, {port}]] &bull; [[admin/development/info:ip, {ip}]]</h3>
		</div>

		<div class="panel-body">
			<span>[[admin/development/info:nodes-responded, {nodeCount}, {timeout}]]</span>

			<table class="table table-striped">
				<thead>
					<tr>
						<td>[[admin/development/info:host]]</td>
						<td class="text-center">[[admin/development/info:primary]]</td>
						<td>[[admin/development/info:pid]]</td>
						<td>[[admin/development/info:nodejs]]</td>
						<td>[[admin/development/info:online]]</td>
						<td>[[admin/development/info:git]]</td>
						<td>[[admin/development/info:cpu-usage]]</td>
						<td>[[admin/development/info:process-memory]]</td>
						<td>[[admin/development/info:system-memory]]</td>
						<td>[[admin/development/info:load]]</td>
						<td>[[admin/development/info:uptime]]</td>
					</tr>
				</thead>
				<tbody>
				<!-- BEGIN info -->
				<tr>
					<td>{info.os.hostname}:{info.process.port}</td>
					<td class="text-center">
						{{{if info.nodebb.isPrimary}}}<i class="fa fa-check"></i>{{{else}}}<i class="fa fa-times"></i>{{{end}}} /
						{{{if info.nodebb.runJobs}}}<i class="fa fa-check"></i>{{{else}}}<i class="fa fa-times"></i>{{{end}}}
					</td>
					<td>{info.process.pid}</td>
					<td>{info.process.version}</td>
					<td>
						<span title="[[admin/development/info:registered]]">{info.stats.onlineRegisteredCount}</span> /
						<span title="[[admin/development/info:guests]]">{info.stats.onlineGuestCount}</span> /
						<span title="[[admin/development/info:sockets]]">{info.stats.socketCount}</span>
					</td>
					<td>{info.git.branch}@<a href="https://github.com/NodeBB/NodeBB/commit/{info.git.hash}" target="_blank">{info.git.hashShort}</a></td>
					<td>{info.process.cpuUsage}%</td>
					<td>
						<span title="[[admin/development/info:used-memory-process]]">{info.process.memoryUsage.humanReadable} gb</span>
					</td>
					<td>
						<span title="[[admin/development/info:used-memory-os]]">{info.os.usedmem} gb</span> /
						<span title="[[admin/development/info:total-memory-os]]">{info.os.totalmem} gb</span>
					</td>
					<td>{info.os.load}</td>
					<td>{info.process.uptimeHumanReadable}</td>
				</tr>
				<!-- END info -->
				</tbody>
			</table>
			</div>
		</div>
	</div>

	<div class="panel panel-default">
		<div class="panel-heading">
			<h3 class="panel-title">[[admin/development/info:info]]</h3>
		</div>

		<div class="panel-body">
			<div class="highlight">
				<pre>{infoJSON}</pre>
			</div>
		</div>
	</div>
</div>