using LegoTrainManager.Models;
using Microsoft.AspNetCore.Identity.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore;

namespace LegoTrainManager.Data
{
    public class ApplicationDbContext : IdentityDbContext
    {
        public DbSet<Train> Trains { get; set; }

        public ApplicationDbContext(DbContextOptions<ApplicationDbContext> options)
            : base(options)
        {
            
        }

        //protected override void OnModelCreating(ModelBuilder builder)
        //{
        //    //builder.Entity<Train>();


        //    //base.OnModelCreating(builder);
        //}
    }
}