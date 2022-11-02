<div class="dashboard">
	<div class="col-xs-12">
		<a class="btn btn-link" href="{config.relative_path}/admin/dashboard">
			<i class="fa fa-chevron-left"></i>
			[[admin/dashboard:back-to-dashboard]]
		</a>

		<!-- IMPORT admin/partials/dashboard/graph.tpl -->
		<!-- IMPORT admin/partials/dashboard/stats.tpl -->

		<table class="table table-striped users-list">
			<thead>
				<th class="text-muted">[[admin/manage/users:users.uid]]</th>
				<th class="text-muted">[[admin/manage/users:users.username]]</th>
				<th class="text-muted">[[admin/manage/users:users.email]]</th>
				<th data-sort="joindate">[[admin/manage/users:users.joined]]</th>
			</thead>
			<tbody>
				{{{ if !users.length}}}
				<tr>
					<td colspan=4" class="text-center"><em>[[admin/dashboard:details.no-logins]]</em></td>
				</tr>
				{{{ end }}}
				{{{ each users }}}
				<tr>
					<td>{../uid}</td>
					<td>{../username}</td>
					<td>{../email}</td>
					<td><span class="timeago" title="{../joindateISO}"></span></td>
				</tr>
				{{{ end }}}
			</tbody>
		</table>
	</div>
</div>