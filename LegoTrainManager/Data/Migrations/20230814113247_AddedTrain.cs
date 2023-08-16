using System;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace LegoTrainManager.Data.Migrations
{
    public partial class AddedTrain : Migration
    {
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.CreateTable(
                name: "Trains",
                columns: table => new
                {
                    Id = table.Column<int>(type: "int", nullable: false)
                        .Annotation("SqlServer:Identity", "1, 1"),
                    Name = table.Column<string>(type: "nvarchar(max)", nullable: false),
                    BatteryVoltage = table.Column<double>(type: "float", nullable: false),
                    LastConnectionTime = table.Column<DateTime>(type: "datetime2", nullable: false),
                    Online = table.Column<bool>(type: "bit", nullable: false),
                    DeviceType = table.Column<int>(type: "int", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_Trains", x => x.Id);
                });

            migrationBuilder.CreateTable(
                name: "Powerconsumer",
                columns: table => new
                {
                    Id = table.Column<int>(type: "int", nullable: false)
                        .Annotation("SqlServer:Identity", "1, 1"),
                    Name = table.Column<string>(type: "nvarchar(max)", nullable: false),
                    PowerPercentage = table.Column<int>(type: "int", nullable: false),
                    ConsumerType = table.Column<int>(type: "int", nullable: false),
                    TrainId = table.Column<int>(type: "int", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_Powerconsumer", x => x.Id);
                    table.ForeignKey(
                        name: "FK_Powerconsumer_Trains_TrainId",
                        column: x => x.TrainId,
                        principalTable: "Trains",
                        principalColumn: "Id");
                });

            migrationBuilder.CreateIndex(
                name: "IX_Powerconsumer_TrainId",
                table: "Powerconsumer",
                column: "TrainId");
        }

        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropTable(
                name: "Powerconsumer");

            migrationBuilder.DropTable(
                name: "Trains");
        }
    }
}
