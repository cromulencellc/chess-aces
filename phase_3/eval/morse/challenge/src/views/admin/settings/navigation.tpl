<div class="row" id="navigation">
	<div class="col-lg-9">
		<div class="clearfix">
			<ul id="active-navigation" class="nav navbar-nav">
				<!-- BEGIN navigation -->
				<li data-index="{navigation.index}" class="{navigation.class} {{{ if navigation.selected }}} active {{{ end }}}">
					<a href="#" title="{navigation.route}" id="{navigation.id}">
						<i class="nav-icon fa fa-fw {{{ if navigation.iconClass }}}{navigation.iconClass}{{{ end }}}"></i><i class="dropdown-icon fa fa-caret-down{{{ if !navigation.dropdown }}} hidden{{{ end }}}"></i>
					</a>
				</li>
				<!-- END navigation -->
			</ul>
		</div>

		<hr/>

		<ul id="enabled">
			{{{ each enabled }}}
			<li data-index="{enabled.index}" class="well <!-- IF !enabled.selected -->hidden<!-- ENDIF !enabled.selected -->">
				<form>
					<div class="row">
						<div class="col-sm-1">
							<div class="form-group">
								<label>[[admin/settings/navigation:icon]]</label>
								<br/>
								<span class="iconPicker"><i class="fa fa-2x {enabled.iconClass}"></i>
									<a class="change-icon-link <!-- IF enabled.iconClass -->hidden<!-- ENDIF enabled.iconClass -->" href="#">[[admin/settings/navigation:change-icon]]</a>
									<input class="form-control" type="hidden" name="iconClass" value="{enabled.iconClass}" />
								</span>
							</div>
						</div>

						<div class="col-sm-3">
							<div class="form-group">
								<label for="nav:route">[[admin/settings/navigation:route]]</label>
								<input id="nav:route" class="form-control" type="text" name="route" value="{enabled.route}" />
							</div>
						</div>

						<div class="col-sm-4">
							<div class="form-group">
								<label for="nav:class">[[admin/settings/navigation:class]]</label>
								<input id="nav:class" class="form-control" type="text" name="class" value="{enabled.class}" />
							</div>
						</div>

						<div class="col-sm-4">
							<div class="form-group">
								<label for="nav:id">[[admin/settings/navigation:id]]</label>
								<input id="nav:id" class="form-control" type="text" name="id" value="{enabled.id}" />
							</div>
						</div>
					</div>
					<div class="row">
						<div class="col-sm-4">
							<div class="form-group">
								<label for="nav:text">[[admin/settings/navigation:text]]</label>
								<input id="nav:text" class="form-control unescape" type="text" name="text" value="{enabled.text}" />
							</div>
						</div>
						<div class="col-sm-4">
							<div class="form-group">
								<label for="nav:text-class">[[admin/settings/navigation:text-class]]</label>
								<input id="nav:text-class" class="form-control" type="text" name="textClass" value="{enabled.textClass}" />
							</div>
						</div>

						<div class="col-sm-4">
							<div class="form-group">
								<label for="nav:tooltip">[[admin/settings/navigation:tooltip]]</label>
								<input id="nav:tooltip" class="form-control unescape" type="text" name="title" value="{enabled.title}" />
							</div>
						</div>
					</div>

					<strong>[[admin/settings/navigation:groups]]</strong>
					<div>
						<select name="groups" class="form-control" size="10" multiple>
							{{{ each enabled.groups }}}
							<option value="{enabled.groups.displayName}"<!-- IF enabled.groups.selected --> selected<!-- ENDIF enabled.groups.selected -->>{enabled.groups.displayName}</option>
							{{{ end }}}
						</select>
					</div>

					<div class="checkbox">
						<label class="mdl-switch mdl-js-switch mdl-js-ripple-effect">
							<input class="mdl-switch__input" type="checkbox" name="targetBlank" <!-- IF enabled.targetBlank -->checked<!-- ENDIF enabled.targetBlank -->/>
							<span class="mdl-switch__label"><strong>[[admin/settings/navigation:open-new-window]]</strong></span>
						</label>
					</div>

					<div class="checkbox">
						<label class="mdl-switch mdl-js-switch mdl-js-ripple-effect">
							<input class="mdl-switch__input" type="checkbox" name="dropdown" {{{ if enabled.dropdown }}}checked{{{ end }}}/>
							<span class="mdl-switch__label"><strong>[[admin/settings/navigation:dropdown]]</strong></span>
						</label>
					</div>
					<div>
						<p class="help-block">
							[[admin/settings/navigation:dropdown-placeholder]]
						</p>
						<textarea name="dropdownContent" rows="5" class="form-control">{enabled.dropdownContent}</textarea>
					</div>

					<button class="btn btn-danger delete">[[admin/settings/navigation:btn.delete]]</button>
					<!-- IF enabled.enabled -->
					<button class="btn btn-warning toggle">[[admin/settings/navigation:btn.disable]]</button>
					<!-- ELSE -->
					<button class="btn btn-success toggle">[[admin/settings/navigation:btn.enable]]</button>
					<!-- ENDIF enabled.enabled -->
					<input type="hidden" name="enabled" value="{enabled.enabled}" />
				</form>
			</li>
			{{{ end }}}
		</ul>
	</div>

	<div class="col-lg-3">
		<div class="panel panel-default">
			<div class="panel-heading">[[admin/settings/navigation:available-menu-items]]</div>
			<div class="panel-body">
				<ul id="available">
					<li data-id="custom" class="clearfix">
						<div data-id="custom" class="drag-item alert alert-success pull-left">
							<i class="fa fa-fw fa-plus-circle"></i>
						</div>
						<p>
							<strong>[[admin/settings/navigation:custom-route]]</strong>
						</p>
					</li>
					{{{ each available }}}
					<li data-id="{@index}" class="clearfix">
						<div data-id="{@index}" class="drag-item alert <!-- IF available.core -->alert-warning<!-- ELSE -->alert-info<!-- ENDIF available.core --> pull-left">
							<i class="fa fa-fw <!-- IF available.iconClass -->{available.iconClass}<!-- ELSE -->fa-navicon<!-- ENDIF available.iconClass -->"></i>
						</div>
						<p>
							<strong>{available.text}</strong> {available.route} <br/>
							<!-- IF available.core --> [[admin/settings/navigation:core]] <!-- ELSE --> [[admin/settings/navigation:plugin]] <!-- ENDIF available.core -->
						</p>
					</li>
					{{{ end }}}
				</ul>
			</div>
		</div>
	</div>
</div>

<button id="save" class="floating-button mdl-button mdl-js-button mdl-button--fab mdl-js-ripple-effect mdl-button--colored">
	<i class="material-icons">save</i>
</button>