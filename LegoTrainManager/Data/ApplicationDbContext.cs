using LegoTrainManager.Models;
using Microsoft.AspNetCore.Identity.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore;

namespace LegoTrainManager.Data
{
    public class ApplicationDbContext : IdentityDbContext
    {

        public DbSet<Train> Trains { get; set; }
        //private static DbContextOptions<ApplicationDbContext> _options;
        public ApplicationDbContext(DbContextOptions<ApplicationDbContext> options)
            : base(options)
        {
            //_options= options;
        }

        protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder)
        {
            optionsBuilder.UseMySQL("server=192.168.0.207; Port=30306;database=LegoManager;user=root;password=pmoelans84");
        }



        //protected override void OnModelCreating(ModelBuilder builder)
        //{
        //    //builder.Entity<Train>();


        //    //base.OnModelCreating(builder);
        //}
    }
}