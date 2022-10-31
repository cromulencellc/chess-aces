<!-- IMPORT admin/partials/settings/header.tpl -->

<div class="row">
	<div class="col-sm-2 col-xs-12 settings-header">
		[[admin/settings/general:site-settings]]
	</div>
	<div class="col-sm-10 col-xs-12">
		<form>
			<label for="site-title">[[admin/settings/general:title]]</label>
			<input id="site-title" class="form-control" type="text" placeholder="[[admin/settings/general:title.name]]" data-field="title" />
			<label for="title:short">[[admin/settings/general:title.short]]</label>
			<input id="title:short" type="text" class="form-control" placeholder="[[admin/settings/general:title.short-placeholder]]" data-field="title:short" />
			<label for="title:url">[[admin/settings/general:title.url]]</label>
			<input id ="title:url" type="text" class="form-control" placeholder="[[admin/settings/general:title.url-placeholder]]" data-field="title:url" />
			<p class="help-block">
				[[admin/settings/general:title.url-help]]
			</p>

			<div class="checkbox">
				<label for="showSiteTitle" class="mdl-switch mdl-js-switch mdl-js-ripple-effect">
					<input type="checkbox" class="mdl-switch__input" id="showSiteTitle" data-field="showSiteTitle" name="showSiteTitle" />
					<span class="mdl-switch__label">[[admin/settings/general:title.show-in-header]]</span>
				</label>
			</div>

			<label for="browserTitle">[[admin/settings/general:browser-title]]</label>
			<input id="browserTitle" class="form-control" type="text" placeholder="[[admin/settings/general:browser-title]]" data-field="browserTitle" />
			<p class="help-block">
				[[admin/settings/general:browser-title-help]]
			</p>

			<label for="titleLayout">[[admin/settings/general:title-layout]]</label>
			<input id="titleLayout" class="form-control" type="text" placeholder="[[admin/settings/general:title-layout]]" data-field="titleLayout" />
			<p class="help-block">
				[[admin/settings/general:title-layout-help]]
			</p>

			<label for="description">[[admin/settings/general:description]]</label>
			<input id="description" type="text" class="form-control" placeholder="[[admin/settings/general:description.placeholder]]" data-field="description" /><br />

			<label for="keywords">[[admin/settings/general:keywords]]</label><br />
			<input id="keywords" type="text" class="form-control" placeholder="[[admin/settings/general:keywords-placeholder]]" data-field="keywords" data-field-type="tagsinput" /><br />
		</form>
	</div>
</div>

<div class="row">
	<div class="col-sm-2 col-xs-12 settings-header">[[admin/settings/general:logo]]</div>
	<div class="col-sm-10 col-xs-12">
		<div class="form-group">
			<label for="logoUrl">[[admin/settings/general:logo.image]]</label>
			<div class="input-group">
				<input id="logoUrl" type="text" class="form-control" placeholder="[[admin/settings/general:logo.image-placeholder]]" data-field="brand:logo" />
				<span class="input-group-btn">
					<input data-action="upload" data-target="logoUrl" data-route="{config.relative_path}/api/admin/uploadlogo" type="button" class="btn btn-default" value="[[admin/settings/general:logo.upload]]" />
					<button data-action="removeLogo" type="button" class="btn btn-default btn-danger"><i class="fa fa-times"></i></button>
				</span>
			</div>
		</div>

		<div class="form-group">
			<label for="brand:logo:url">[[admin/settings/general:logo.url]]</label>
			<input id ="brand:logo:url" type="text" class="form-control" placeholder="[[admin/settings/general:logo.url-placeholder]]" data-field="brand:logo:url" />
			<p class="help-block">
				[[admin/settings/general:logo.url-help]]
			</p>
		</div>
		<div class="form-group">
			<label for="brand:logo:alt">[[admin/settings/general:logo.alt-text]]</label>
			<input id ="brand:logo:alt" type="text" class="form-control" placeholder="[[admin/settings/general:log.alt-text-placeholder]]" data-field="brand:logo:alt" />
		</div>

		<div class="form-group">
			<label for="og_image">og:image</label>
			<div class="input-group">
				<input id="og_image" type="text" class="form-control" placeholder="" data-field="og:image" />
				<span class="input-group-btn">
					<input data-action="upload" data-target="og_image" data-route="{config.relative_path}/api/admin/uploadOgImage" type="button" class="btn btn-default" value="[[admin/settings/general:logo.upload]]" />
					<button data-action="removeOgImage" type="button" class="btn btn-default btn-danger"><i class="fa fa-times"></i></button>
				</span>
			</div>
		</div>
	</div>
</div>

<div class="row">
	<div class="col-sm-2 col-xs-12 settings-header">
		[[admin/settings/general:favicon]]
	</div>
	<div class="col-sm-10 col-xs-12">
		<div class="form-group">
			<div class="input-group">
				<input id="faviconUrl" type="text" class="form-control" placeholder="favicon.ico" data-field="brand:favicon" data-action="upload" data-target="faviconUrl" data-route="{config.relative_path}/api/admin/uploadfavicon" readonly />
				<span class="input-group-btn">
					<input data-action="upload" data-target="faviconUrl" data-route="{config.relative_path}/api/admin/uploadfavicon" data-help="0" type="button" class="btn btn-default" value="[[admin/settings/general:favicon.upload]]" />
					<button data-action="removeFavicon" type="button" class="btn btn-default btn-danger"><i class="fa fa-times"></i></button>
				</span>
			</div>
		</div>
	</div>
</div>

<div class="row">
	<div class="col-sm-2 col-xs-12 settings-header">
		[[admin/settings/general:pwa]]
	</div>
	<div class="col-sm-10 col-xs-12">
		<div class="form-group">
			<label for="touchIconUrl">[[admin/settings/general:touch-icon]]</label>
			<div class="input-group">
				<input id="touchIconUrl" type="text" class="form-control" data-field="brand:touchIcon" data-action="upload" data-target="touchIconUrl" data-route="{config.relative_path}/api/admin/uploadTouchIcon" readonly />
				<span class="input-group-btn">
					<input data-action="upload" data-target="touchIconUrl" data-route="{config.relative_path}/api/admin/uploadTouchIcon" type="button" class="btn btn-default" value="[[admin/settings/general:touch-icon.upload]]" />
					<button data-action="removeTouchIcon" type="button" class="btn btn-default btn-danger"><i class="fa fa-times"></i></button>
				</span>
			</div>
			<p class="help-block">
				[[admin/settings/general:touch-icon.help]]
			</p>
		</div>

		<div class="form-group">
			<label for="maskableIconUrl">[[admin/settings/general:maskable-icon]]</label>
			<div class="input-group">
				<input id="maskableIconUrl" type="text" class="form-control" data-field="brand:maskableIcon" data-action="upload" data-target="maskableIconUrl" data-route="{config.relative_path}/api/admin/uploadMaskableIcon" readonly />
				<span class="input-group-btn">
					<input data-action="upload" data-target="maskableIconUrl" data-route="{config.relative_path}/api/admin/uploadMaskableIcon" type="button" class="btn btn-default" value="[[admin/settings/general:touch-icon.upload]]" />
					<button data-action="removeMaskableIcon" type="button" class="btn btn-default btn-danger"><i class="fa fa-times"></i></button>
				</span>
			</div>
			<p class="help-block">
				[[admin/settings/general:maskable-icon.help]]
			</p>
		</div>
	</div>
</div>

<div class="row">
	<div class="col-sm-2 col-xs-12 settings-header">[[admin/settings/general:search]]</div>
	<div class="col-sm-10 col-xs-12">
		<div class="form-group">
			<label for="searchDefaultIn">[[admin/settings/general:search-default-in]]</label>
			<select id="searchDefaultIn" class="form-control" data-field="searchDefaultIn">
				<option value="titlesposts">[[search:titles-posts]]</option>
				<option value="titles">[[search:titles]]</option>
				<option value="posts">[[global:posts]]</option>
				<option value="categories">[[global:header.categories]]</option>
				<option value="users">[[global:users]]</option>
				<option value="tags">[[tags:tags]]</option>
			</select>
		</div>
		<div class="form-group">
			<label for="searchDefaultInQuick">[[admin/settings/general:search-default-in-quick]]</label>
			<select id="searchDefaultInQuick" class="form-control" data-field="searchDefaultInQuick">
				<option value="titlesposts">[[search:titles-posts]]</option>
				<option value="titles">[[search:titles]]</option>
				<option value="posts">[[global:posts]]</option>
			</select>
		</div>
		<div class="form-group">
			<label for="post-sort-by">[[admin/settings/general:search-default-sort-by]]</label>
			<select id="post-sort-by" class="form-control" data-field="searchDefaultSortBy">
				<option value="relevance">[[search:relevance]]</option>
				<option value="timestamp">[[search:post-time]]</option>
				<option value="votes">[[search:votes]]</option>
				<option value="topic.lastposttime">[[search:last-reply-time]]</option>
				<option value="topic.title">[[search:topic-title]]</option>
				<option value="topic.postcount">[[search:number-of-replies]]</option>
				<option value="topic.viewcount">[[search:number-of-views]]</option>
				<option value="topic.votes">[[search:topic-votes]]</option>
				<option value="topic.timestamp">[[search:topic-start-date]]</option>
				<option value="user.username">[[search:username]]</option>
				<option value="category.name">[[search:category]]</option>
			</select>
		</div>
	</div>
</div>

<div class="row">
	<div class="col-sm-2 col-xs-12 settings-header">[[admin/settings/general:outgoing-links]]</div>
	<div class="col-sm-10 col-xs-12">
		<form>
			<div class="checkbox">
				<label for="useOutgoingLinksPage" class="mdl-switch mdl-js-switch mdl-js-ripple-effect">
					<input type="checkbox" class="mdl-switch__input" id="useOutgoingLinksPage" data-field="useOutgoingLinksPage">
					<span class="mdl-switch__label"><strong>[[admin/settings/general:outgoing-links.warning-page]]</strong></span>
				</label>
			</div>

			<div class="form-group">
				<label for="outgoingLinks:whitelist">[[admin/settings/general:outgoing-links.whitelist]]</label><br />
				<input id="outgoingLinks:whitelist" type="text" class="form-control" placeholder="subdomain.domain.com" data-field="outgoingLinks:whitelist" data-field-type="tagsinput" />
			</div>
		</form>
	</div>
</div>

<div class="row">
	<div class="col-sm-2 col-xs-12 settings-header">[[admin/settings/general:site-colors]]</div>
	<div class="col-sm-10 col-xs-12">
		<form>
			<label for="themeColor">[[admin/settings/general:theme-color]]</label>
			<input id="themeColor" type="text" class="form-control" placeholder="#ffffff" data-field="themeColor" />

			<label for="backgroundColor">[[admin/settings/general:background-color]]</label>
			<input id="backgroundColor" type="text" class="form-control" placeholder="#ffffff" data-field="backgroundColor" />
			<p class="help-block">
				[[admin/settings/general:background-color-help]]
			</p>
		</form>
	</div>
</div>

<div class="row">
	<div class="col-sm-2 col-xs-12 settings-header">[[admin/settings/general:topic-tools]]</div>
	<div class="col-sm-10 col-xs-12">
		<form>
			<label for="undoTimeout">[[admin/settings/general:undo-timeout]]</label>
			<input id="undoTimeout" type="text" class="form-control" placeholder="10000" data-field="undoTimeout" />
			<p class="help-block">
				[[admin/settings/general:undo-timeout-help]]
			</p>
		</form>
	</div>
</div>
<!-- IMPORT admin/partials/settings/footer.tpl -->