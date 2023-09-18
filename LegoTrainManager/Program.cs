using LegoTrainManager.Controllers;
using LegoTrainManager.Data;
using LegoTrainManager.Models;
using LegoTrainManager.Services;
using Microsoft.AspNetCore.Identity;

var builder = WebApplication.CreateBuilder(args);
var cancellationTokenProvider = new CancellationTokenSource();
builder.Services.AddSingleton<IServiceProvider, ServiceProvider>();
builder.Services.AddScoped<IMqttMessageHandler, StatusHandler>();
builder.Services.AddScoped<ITrainSearchEngine,TrainSearchEngine>();
builder.Services.AddSingleton<IMQTTTrainControlHandler, MQTTTrainControlHandlerService>();
builder.Services.AddTransient<TrainsController>();

// Add services to the container.
//var connectionString = builder.Configuration.GetConnectionString("DefaultConnection");
//builder.Services.AddTransient<MySqlConnection>(_ =>
//    new MySqlConnection(connectionString));
builder.Services.AddDbContext<ApplicationDbContext>(//options=>
        
//        options.UseMySql(connectionString,b=>b.EnableRetryOnFailure())
//       // ServiceLifetime.Scoped

    
);

builder.Services.AddDatabaseDeveloperPageExceptionFilter();

builder.Services.AddDefaultIdentity<IdentityUser>(options => options.SignIn.RequireConfirmedAccount = true)
    .AddEntityFrameworkStores<ApplicationDbContext>();
builder.Services.AddControllersWithViews();


//builder.Services.AddSingleton<IConfiguration>(Configuration);


var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseMigrationsEndPoint();
}
else
{
    app.UseExceptionHandler("/Home/Error");
    // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
    app.UseHsts();
}
using (var serviceScope = app.Services.CreateScope())
{
    var services = serviceScope.ServiceProvider;
    var myTrainController = services.GetRequiredService<TrainsController>();

    var myDependency = services.GetRequiredService<IMQTTTrainControlHandler>();
    myDependency.StartAsync(cancellationTokenProvider.Token);

    myDependency.AddMessageHandler(new StatusHandler(myTrainController));
}


app.UseHttpsRedirection();
app.UseStaticFiles();
//app.UseMiddleware();
app.UseRouting();

app.UseAuthentication();
app.UseAuthorization();

app.MapControllerRoute(
    name: "default",
    pattern: "{controller=Home}/{action=Index}/{id?}");
app.MapRazorPages();

app.Run();
