<html>
<head>
<style>

/* Devils Stone: #226E2D */
/* a2b6a5: #A2B6A5 */
/* D5DCD6: #D5DCD6 */
/* lullaby: #D9EADB */
/* green chromoxice lt: #DBF9EF */

/*
*/
* {
	padding: 0;
	margin: 0;
	}

/*
div#main p, div#main ul, div#main li, div#main div {
	padding: .2em;
	margin: .2em;
	}
*/

body {
	padding: 0;
	margin: 0;
	background-color: #d3c892;
	}

div#page {
	padding: 0;
	margin: 0;
	}

div#navbar {
	width: 100%;
	background-color: #504e52;
	background: linear-gradient(top, #504e52; 0%, #29272c 100%);  
	background: -moz-linear-gradient(top, #504e52 0%, #29272c 100%); 
	background: -webkit-linear-gradient(top, #504e52 0%, #29272c 100%); 
	box-shadow: 0px 1px 5px black;
	padding: 0;
	margin: 0;
	z-index: 30;

	/* Firefox */
	display:-moz-box;
	-moz-box-pack:center;
	-moz-box-align:center;

	/* Safari, Opera, and Chrome */
	display:-webkit-box;
	-webkit-box-pack:center;
	-webkit-box-align:center;

	/* W3C */
	display:box;
	box-pack:center;
	box-align:center;
	}

div#main {
	padding: .5em 1em .2em 1em;
	margin: 0;
	width: 80%;
	position: relative;
	left: 50%;
	/* margin-top: -.5em; */
	margin-left: -40%;
	background-color: #A0A0A0;
	color: black;
	box-shadow: 0px 0px 10px black;
	z-index: 20;
	}

div#main p {
	padding: 0.2em;
	line-height: 1.35em;
	}


/* Styling menu */
nav ul {
	margin: 0;
	background: #504e52; 
	background: linear-gradient(top, #504e52; 0%, #29272c 100%);  
	background: -moz-linear-gradient(top, #504e52 0%, #29272c 100%); 
	background: -webkit-linear-gradient(top, #504e52 0%, #29272c 100%); 
	padding: 0;
	list-style: none;
	position: relative;
	display: inline-table;

	z-index: 40;
	}

nav ul:after {
	content: ""; 
	clear: both; 
	display: block;
	}

nav ul li {
	float: left;
	}

nav ul li:hover { 						/* change colors on hover */
	background: #e0e1dc;
	background: linear-gradient(top, #e0e1dc 0%, #b7b8b4 40%);
	background: -moz-linear-gradient(top, #e0e1dc 0%, #b7b8b4 40%);
	background: -webkit-linear-gradient(top, #e0e1dc 0%, #b7b8b4 40%);
	}

/*
nav ul li a,
nav ul ul li a {
*/
nav ul li a {
	padding: 1em 1.2em;
	text-decoration: none;
	display: block; 
	color: white;
	border-left: 1px solid #636266;
	/* border-right: 1px solid #888; */
	}

nav ul ul li:first-child a {
	border: 0;
	}

nav ul li:hover a {
	color: black;
	}



/* The logo */
nav ul li#logo a {
	width: 72px;
	min-height: 50px;
	padding: 0;
	padding-right: .4em;
	border: 0;
	background:url(web_v003b.png) 0px 0px;
	background-repeat:no-repeat;
	}

/*
	*/
nav ul li#logo:hover {
	background-color: #504e52;
	background: linear-gradient(top, #504e52; 0%, #29272c 100%);  
	background: -moz-linear-gradient(top, #504e52 0%, #29272c 100%); 
	background: -webkit-linear-gradient(top, #504e52 0%, #29272c 100%); 
	}

nav ul li#logo:hover a {
	background:url(web_v003b.png) 0px -100px;
	background-repeat:no-repeat;
	}

/* Styling submenus */
nav ul ul {
	background-color: #3f3d42;; 
	box-shadow: 0px 2px 5px black;
	position: absolute; 
	top: 100%;
	}

nav ul ul li {
	float: none; 
	position: relative;
	}

nav ul ul li a {
	border: 0;
	padding: 1em 1.2em;
	border-top: 1px solid #626064;;
	border-bottom: 1px solid #3f3d42;;
	color: white;
	}	

nav ul li:hover ul li a:hover {
	color: black;
	}

nav ul li:hover ul li a {
	color: white;
	}

/* Moving the sub-submenu */
nav ul ul ul {
	position: absolute; 
	left: 100%; 
	top: 0;
	}

/* Hide submenus */
nav ul ul { 
	display: none; 
	}

/* Show submenus on hover*/
nav ul li:hover > ul { 
	display: block; 
	}

</style>
</head>

<body>
<div id="page">

<div id="navbar">

	<!--
	-->
	<nav>
	<ul>
		<li id="logo"><a href="#"></a></li>
		<!--
		<li id="logo"><a href="#"><img src="./web_v001.png"></a></li>
		-->
		<li><a href="#">Sensors</a>
			<ul>
				<li><a href="#">History</a></li>
				<li><a href="#">Compendium</a></li>
				<li><a href="#">Other Thing</a>
					<ul>
						<li><a href="#">wut wut</a></li>
						<li><a href="#">are f'n kidding</a></li>
						<li><a href="#">that's right yo</a></li>
					</ul>
				</li>
			</ul>
		</li>
		<li><a href="#">Panels</a>
			<ul>
				<li><a href="#">alpha</a></li>
				<li><a href="#">two</a></li>
				<li><a href="#">**Add new panel</a></li>
			</ul>
		</li>
		<li><a href="#">Tools</a>
			<ul>
				<li><a href="#">Settings</a></li>
				<li><a href="#">Triggers</a></li>
				<li><a href="#">Admin</a></li>
				<li><a href="#">Mate</a></li>
			</ul>
		</li>
		<li><a href="#">Help</a>
			<ul>
				<li><a href="#">User's Guide</a></li>
				<li><a href="#">Programmer's Guide</a></li>
			</ul>
		</li>
		<li><a href="#">Log Out</a></li>
	</ul>
	</nav>
	<!--
	-->
</div> 

<!--
<div id="nav">
</div>
-->
<div id="main">
<p style="text-align: right;">
2013-Mar-08 06:55
 
</p>

<h1>Main</h1>

<p>
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec porttitor, lorem in auctor gravida, neque risus varius lectus, sed scelerisque tellus ligula ut mi. Proin sit amet nulla aliquam mi eleifend fermentum at accumsan arcu. In nulla metus, ornare vel faucibus ac, aliquam rhoncus dui. Cras dictum dignissim lobortis. Maecenas vel nisl ac leo lobortis adipiscing. Vestibulum eu felis at odio commodo pretium nec in nulla. Pellentesque vel dui in lorem dapibus pharetra. In a cursus eros. Cras sodales lectus non quam dictum tristique. Nulla facilisi.
</p>

<p>
Pellentesque egestas massa neque, et sodales magna. Nunc eget orci enim. Praesent pharetra lobortis eros, vitae lacinia sem posuere sit amet. Vivamus ac leo est. Nam iaculis eros sollicitudin massa ullamcorper viverra. Phasellus tincidunt ornare venenatis. Nulla facilisi. Nulla ornare tincidunt sollicitudin. Ut vitae neque eu nulla ullamcorper suscipit. Aliquam et lacinia diam. Suspendisse a enim eros, aliquet tristique lectus. Nulla et felis vel ipsum vestibulum scelerisque ut vitae dolor. Aliquam hendrerit cursus augue in vulputate.
</p>

<p>
Morbi dignissim lectus at eros facilisis et sollicitudin arcu auctor. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Aenean nisi lorem, suscipit eget ultricies et, interdum et turpis. Integer suscipit sem eget felis dignissim tincidunt. Quisque a nibh nunc. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Sed auctor porta leo, et tincidunt turpis posuere sed. Proin magna enim, ultricies nec aliquet id, rhoncus nec nibh. Quisque sit amet justo in quam varius pharetra.
</p>

<p>
Fusce eros est, semper id sollicitudin eu, laoreet a velit. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Vivamus dictum quam at urna eleifend vel dictum lacus blandit. Duis in nisl neque, eget faucibus tellus. Donec fermentum orci sit amet sapien sodales sit amet feugiat sem blandit. Quisque vitae dolor sed arcu fermentum tincidunt non ac lorem. Integer placerat porta dui, sit amet lacinia ligula laoreet mollis. Maecenas elementum urna sit amet lacus tristique ac sollicitudin mauris ornare.
</p>

<p>
Curabitur ac vulputate elit. Cras urna dui, eleifend sed tempus quis, imperdiet ac orci. Aliquam erat volutpat. Quisque accumsan augue nec neque tempus gravida. Phasellus sit amet ornare sapien. Curabitur ultrices quam sit amet erat feugiat sed gravida neque varius. Duis venenatis laoreet nulla, non fermentum elit molestie interdum. Nam vitae lectus quis arcu tristique ultricies id ac lectus. Nulla ornare consequat nulla, ac dignissim felis consequat ac. Quisque neque ante, iaculis sit amet vestibulum sagittis, cursus non velit. Proin est dui, vulputate sit amet tincidunt porttitor, ultrices vel tortor. Curabitur ornare egestas sem, quis euismod ipsum eleifend ac. 
</p>
</div>





</div>
</body>
</html>
