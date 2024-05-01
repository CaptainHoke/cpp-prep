#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <memory>

// Small script for converting logical addresses into physical ones

int main()
{
	// ye ye not clean
	// std::ifstream InputFile{ "./../example_dataset.txt" };
	std::ifstream InputFile{ "./../phys_mem_dataset.txt" };

	if (!InputFile.is_open())
	{
		std::cout << "Failed to open dataset" << std::endl;
		return 1;
	}

	std::ofstream OutputFile{ "./../output.txt" };
	if (!OutputFile.is_open())
	{
		std::cout << "Failed to open output file" << std::endl;
		return 1;
	}

	uint64_t PhysMemAddressCount, Queries, CR3Address;
	InputFile >> PhysMemAddressCount >> Queries >> CR3Address;

	std::unordered_map<uint64_t, uint64_t> PhysMem;

	for (int i = 0; i < PhysMemAddressCount; ++i)
	{
		uint64_t Address, Value;
		InputFile >> Address >> Value;

		PhysMem[Address] = Value;
	}

	auto TranslateAddress = [&](uint64_t LogicalAddr, uint64_t& OutPhysAddr) -> bool {
		uint64_t CurrentPhysAddress = CR3Address;
		uint64_t CurrentTableRecord = 0;

		for (int i = 0; i < 4; ++i)
		{
			uint64_t PageIndexMask = 0x1ffull << (39 - i * 9);
			uint64_t AddressTableIndex = (LogicalAddr & PageIndexMask) >> (39 - i * 9);
			uint64_t CurrentAddress = CurrentPhysAddress + AddressTableIndex * 8;
			CurrentTableRecord = PhysMem[CurrentAddress];

			if ((CurrentTableRecord & 1) == 0) // P bit isn't set => unused table record
			{
				return false;
			}
			CurrentTableRecord -= 1; // Clear P bit

			constexpr uint64_t TableRecordPhysAddressMask = 0xffffffffff000;
			CurrentPhysAddress = CurrentTableRecord & TableRecordPhysAddressMask;

			// Get index into the next address table
			PageIndexMask >>= 9;
		}

		OutPhysAddr = CurrentPhysAddress + (LogicalAddr & 0b111111111111);
		return true;
	};

	for (int i = 0; i < Queries; ++i)
	{
		uint64_t Query;
		InputFile >> Query;

		uint64_t PhysAddr;
		if (TranslateAddress(Query, PhysAddr))
		{
			OutputFile << PhysAddr;
		}
		else
		{
			OutputFile << "fault";
		}

		OutputFile << std::endl;
	}

	return 0;
}
